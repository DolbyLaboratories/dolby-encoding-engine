#include <fstream>
#include "hevc_dec_ffmpeg_utils.h"

BufferBlob::BufferBlob(const void* data_to_copy, size_t size)
{
    data = new char[size];
    data_size = size;
    memcpy(data, data_to_copy, size);
}

BufferBlob::~BufferBlob()
{
    delete[] data;
}

void remove_pictures(hevc_dec_ffmpeg_data_t* data)
{
    // remove all pictures already sent out during last process
    //if (data->decoded_pictures_to_discard) std::cout << "Pop " << data->decoded_pictures_to_discard << " of " <<  data->decoded_pictures.size() << " frames." << std::endl;

    data->calls++;
    if (0 == data->decoded_pictures_to_discard) data->missed_calls++;
    while (data->decoded_pictures_to_discard > 0)
    {
        if (data->decoded_pictures.front().plane[0]) free(data->decoded_pictures.front().plane[0]);
        if (data->decoded_pictures.front().plane[1]) free(data->decoded_pictures.front().plane[1]);
        if (data->decoded_pictures.front().plane[2]) free(data->decoded_pictures.front().plane[2]);
        data->decoded_pictures.pop_front();
        data->decoded_pictures_to_discard -= 1;
    }

    //double miss_rate = ((double)data->missed_calls/(double)data->calls) * 100.0;
    //std::cout << "Miss rate: " << miss_rate << " (" << data->missed_calls << " / " << data->calls << ")" << std::endl;
}

hevc_dec_frame_rate_t string_to_fr(const std::string& str)
{
    hevc_dec_frame_rate_t fr;
    fr.frame_period = 0;
    fr.time_scale = 0;

    if ("23.976" == str)
    {
        fr.frame_period = 24000;
        fr.time_scale = 1001;
    }
    else if ("24" == str)
    {
        fr.frame_period = 24;
        fr.time_scale = 1;
    }
    else if ("25" == str)
    {
        fr.frame_period = 25;
        fr.time_scale = 1;
    }
    else if ("29.97" == str)
    {
        fr.frame_period = 30000;
        fr.time_scale = 1001;
    }
    else if ("30" == str)
    {
        fr.frame_period = 30;
        fr.time_scale = 1;
    }
    else if ("48" == str)
    {
        fr.frame_period = 48;
        fr.time_scale = 1;
    }
    else if ("50" == str)
    {
        fr.frame_period = 50;
        fr.time_scale = 1;
    }
    else if ("59.94" == str)
    {
        fr.frame_period = 60000;
        fr.time_scale = 1001;
    }
    else if ("60" == str)
    {
        fr.frame_period = 60;
        fr.time_scale = 1;
    }

    return fr;
}

bool bin_exists(const std::string& bin, const std::string& arg)
{
    std::string cmd = bin + " " + arg;
    int rt = system(cmd.c_str());
    return (rt == 0);
}

void run_cmd_thread_func(std::string cmd, hevc_dec_ffmpeg_data_t* decoding_data)
{
    int ret_code = system(cmd.c_str());
    decoding_data->ffmpeg_ret_code = ret_code;
}

void writer_thread_func(hevc_dec_ffmpeg_data_t* decoding_data)
{
    if (decoding_data->in_pipe.connectPipe() != 0)
    {
        return;
    }

    while (decoding_data->stop_writing_thread == false || decoding_data->in_buffer.size() > 0)
    {
        if (decoding_data->force_stop_writing_thread)
        {
            break;
        }

        decoding_data->in_buffer_mutex.lock();
        if (decoding_data->in_buffer.empty())
        {
            decoding_data->in_buffer_mutex.unlock();
            continue;
        }
        BufferBlob* front = decoding_data->in_buffer.front();
        decoding_data->in_buffer.pop_front();
        decoding_data->in_buffer_mutex.unlock();

        char* data_to_write = front->data;
        size_t data_size = front->data_size;
        size_t left_to_write = data_size;

        size_t bytes_written;
        while (left_to_write > 0)
        {
            if (decoding_data->in_pipe.writeToPipe(data_to_write, left_to_write, &bytes_written) != 0)
            {
                // error writing to pipe
                bytes_written = 0;
                decoding_data->force_stop_writing_thread = true;
            }
            left_to_write -= bytes_written;
            data_to_write += bytes_written;
            if (bytes_written == 0 && decoding_data->force_stop_writing_thread) break;
        }
        delete front;
    }

    decoding_data->in_pipe.closePipe();
}

void reader_thread_func(hevc_dec_ffmpeg_data_t* decoding_data)
{
    if (decoding_data->out_pipe.connectPipe() != 0)
    {
        return;
    }

    size_t last_read_size = 0;
    char* read_data = new char[READ_BUFFER_SIZE];

    do
    {
        if (decoding_data->force_stop_reading_thread)
        {
            break;
        }

        if (decoding_data->out_pipe.readFromPipe(read_data, READ_BUFFER_SIZE, &last_read_size) != 0)
        {
            // error reading from pipe
            last_read_size = 0;
            decoding_data->stop_reading_thread = true;
        }

        if (last_read_size > 0)
        {
            decoding_data->out_buffer_mutex.lock();
            decoding_data->out_buffer.push_back(new BufferBlob(read_data, last_read_size));
            decoding_data->out_buffer_mutex.unlock();
        }
    }
    while (decoding_data->stop_reading_thread == false || last_read_size > 0);

    decoding_data->out_pipe.closePipe();

    delete[] read_data;
}

int extract_bytes(std::list<BufferBlob*>& blob_list, size_t byte_num, char* buffer)
{
    size_t bytes_to_read = byte_num;
    size_t pos = 0;

    // make sure we have enough data in the blob list
    size_t blob_list_size = 0;
    std::list<BufferBlob*>::iterator it = blob_list.begin();
    while (blob_list_size < bytes_to_read)
    {
        if (it == blob_list.end())
        {
            // not enough data in blob list!
            return -1;
        }

        blob_list_size += (*it)->data_size;

        it++;
    }

    while (bytes_to_read > 0)
    {
        if (blob_list.size() == 0)
        {
            // not enough data in blob list!
            return -1;
        }

        BufferBlob* temp_blob = blob_list.front();
        blob_list.pop_front();

        size_t blob_size = temp_blob->data_size;
        if (blob_size <= bytes_to_read)
        {
            memcpy(buffer + pos, temp_blob->data, blob_size);
            pos += blob_size;
            bytes_to_read -= blob_size;
        }
        else
        {
            memcpy(buffer + pos, temp_blob->data, bytes_to_read);

            BufferBlob* new_blob = new BufferBlob(temp_blob->data + bytes_to_read, blob_size - bytes_to_read);
            blob_list.push_front(new_blob);

            bytes_to_read = 0;
        }

        delete temp_blob;
    }

    return 0;
}

size_t get_buf_size(std::list<BufferBlob*>& blob_list)
{
    size_t blob_list_size = 0;
    std::list<BufferBlob*>::iterator it;
    for (it = blob_list.begin(); it != blob_list.end(); it++)
    {
        blob_list_size += (*it)->data_size;
    }

    return blob_list_size;
}

std::string run_cmd_get_output(std::string cmd)
{
    char* buffer = new char[READ_BUFFER_SIZE];
    std::string result;

#ifdef WIN32
    std::shared_ptr<FILE> pipe(_popen(cmd.c_str(), "r"), _pclose);
#else
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
#endif

    if (!pipe)
    {
        delete[] buffer;
        return "";
    }
    while (!feof(pipe.get()))
    {
        if (fgets(buffer, READ_BUFFER_SIZE, pipe.get()) != NULL)
            result += buffer;
    }

    delete[] buffer;
    return result;
}

int extract_pictures_from_buffer(hevc_dec_ffmpeg_data_t* data)
{
    data->out_buffer_mutex.lock();

    // ONLY YUV 420 8 and 10 bit supported
    int byte_num = data->output_bitdepth > 8 ? 2 : 1;
    size_t y_size = data->width * data->height * byte_num;
    size_t c_size = (data->width * data->height * byte_num) / 4;

    while (get_buf_size(data->out_buffer) >= y_size + 2 * c_size)
    {
        hevc_dec_picture_t new_pic;
        new_pic.bit_depth = data->output_bitdepth;
        new_pic.chroma_size = c_size;
        new_pic.luma_size = y_size;
        new_pic.color_space = data->chroma_format;
        new_pic.frame_rate = data->frame_rate_ext;
        new_pic.frame_type = FRAME_TYPE_AUTO;
        new_pic.height = data->height;
        new_pic.width = data->width;
        new_pic.matrix_coeffs = 0;
        new_pic.transfer_characteristics = 0;
        new_pic.stride[0] = data->width * byte_num;
        new_pic.stride[1] = (data->width * byte_num) / 2;
        new_pic.stride[2] = (data->width * byte_num) / 2;
        new_pic.plane[0] = malloc(y_size);
        new_pic.plane[1] = malloc(c_size);
        new_pic.plane[2] = malloc(c_size);

        int ret_code = 0;
        ret_code += extract_bytes(data->out_buffer, y_size, (char*)new_pic.plane[0]);
        ret_code += extract_bytes(data->out_buffer, c_size, (char*)new_pic.plane[1]);
        ret_code += extract_bytes(data->out_buffer, c_size, (char*)new_pic.plane[2]);

        if (ret_code != 0)
        {
            free(new_pic.plane[0]);
            free(new_pic.plane[1]);
            free(new_pic.plane[2]);
            data->msg += "\nError reading data from output buffer.";
            return -1;
        }

        data->decoded_pictures.push_back(new_pic);
    }
    data->out_buffer_mutex.unlock();

    return 0;
}

bool
write_cfg_file(hevc_dec_ffmpeg_data_t* data, const std::string& file)
{
    std::ofstream cfg_file(file);
    if (cfg_file.is_open())
    {
        cfg_file << "output_bitdepth=" << data->output_bitdepth << "\n";
        cfg_file << "width=" << data->width << "\n";
        cfg_file << "height=" << data->height << "\n";
        cfg_file << "input_file=" << data->in_pipe.getPath() << "\n";
        cfg_file << "output_file=" << data->out_pipe.getPath() << "\n";
        cfg_file << "ffmpeg_bin=" << data->ffmpeg_bin << "\n";
        cfg_file.close();
        return true;
    }
    else
    {
        return false;
    }
}