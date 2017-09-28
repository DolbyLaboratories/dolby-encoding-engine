/*
* BSD 3-Clause License
*
* Copyright (c) 2017, Dolby Laboratories
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
* * Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*   and/or other materials provided with the distribution.
*
* * Neither the name of the copyright holder nor the names of its
*   contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <cstdlib>
#include "hevc_enc_ffmpeg_utils.h"

#define READ_BUFFER_SIZE 1024

BufferBlob::BufferBlob(void* data_to_copy, size_t size)
{
    data = new char[size];
    data_size = size;
    memcpy(data, data_to_copy, size);
}

BufferBlob::~BufferBlob()
{
    delete[] data;
}

void
init_defaults(hevc_enc_ffmpeg_t* state)
{
    state->data->bit_depth = 0;         /**< Must be set by caller */
    state->data->width = 0;             /**< Must be set by caller */
    state->data->height = 0;            /**< Must be set by caller */
    state->data->frame_rate.clear();    /**< Must be set by caller */
    state->data->color_space = "yuv420p";
    state->data->pass_num = 0;
    state->data->data_rate = 15000;
    state->data->max_output_data = 0;

#ifdef WIN32
    state->data->in_pipe = INVALID_HANDLE_VALUE;
    state->data->out_pipe = INVALID_HANDLE_VALUE;
    state->data->ffmpeg_bin = "ffmpeg.exe";
#else
    state->data->in_pipe = 0;
    state->data->out_pipe = 0;
    state->data->ffmpeg_bin = "ffmpeg";
#endif

    state->data->stop_writing_thread = false;
    state->data->stop_reading_thread = false;
}

void 
run_cmd_thread_func(std::string cmd)
{
    system(cmd.c_str());
}

void 
writer_thread_func(hevc_enc_ffmpeg_data_t* encoding_data)
{
#ifdef WIN32
    if (ConnectNamedPipe(encoding_data->in_pipe, NULL) == FALSE)
    {
        return;
    }
#else
    if ((encoding_data->in_pipe = open(encoding_data->temp_file[0].c_str(), O_WRONLY)) == -1)
    {
        return;
    }
#endif
    
    while (encoding_data->stop_writing_thread == false || encoding_data->in_buffer.size() > 0)
    {
        encoding_data->in_buffer_mutex.lock();
        if (encoding_data->in_buffer.empty())
        {
            encoding_data->in_buffer_mutex.unlock();
            continue;
        }
        BufferBlob* front = encoding_data->in_buffer.front();
        encoding_data->in_buffer_mutex.unlock();
        
        char* data_to_write = front->data;
        size_t data_size = front->data_size;
        size_t left_to_write = data_size;

#ifdef WIN32
        DWORD bytes_written;
        while (left_to_write > 0)
        {
            WriteFile(encoding_data->in_pipe, data_to_write, left_to_write, &bytes_written, NULL);
            left_to_write -= bytes_written;
            data_to_write += bytes_written;
        }
#else
        ssize_t bytes_written;
        while (left_to_write > 0)
        {
            bytes_written = write(encoding_data->in_pipe, data_to_write, left_to_write);
            left_to_write -= bytes_written;
            data_to_write += bytes_written;
        }
#endif

        encoding_data->in_buffer_mutex.lock();
        delete front;
        encoding_data->in_buffer.pop_front();
        encoding_data->in_buffer_mutex.unlock();
    }
    
#ifdef WIN32
    if (encoding_data->in_pipe != INVALID_HANDLE_VALUE)
    {
        DisconnectNamedPipe(encoding_data->in_pipe);
    }
#else
    if (encoding_data->in_pipe != 0)
    {
        close(encoding_data->in_pipe);
    }
#endif
}

void 
reader_thread_func(hevc_enc_ffmpeg_data_t* encoding_data)
{
#ifdef WIN32
    if (ConnectNamedPipe(encoding_data->out_pipe, NULL) == FALSE)
    {
        return;
    }
#else
    if ((encoding_data->out_pipe = open(encoding_data->temp_file[1].c_str(), O_RDONLY)) == -1)
    {
        return;
    }
#endif
    
    size_t last_read_size = 0;

    while (encoding_data->stop_reading_thread == false || last_read_size > 0)
    {
        char read_data[READ_BUFFER_SIZE];

#ifdef WIN32
        DWORD bytes_read;
        ReadFile(encoding_data->out_pipe, read_data, READ_BUFFER_SIZE, &bytes_read, NULL);
        last_read_size = bytes_read;
#else
        last_read_size = read(encoding_data->out_pipe, read_data, READ_BUFFER_SIZE);
#endif

        if (last_read_size > 0)
        {
            encoding_data->out_buffer_mutex.lock();
            encoding_data->out_buffer.push_back(new BufferBlob(read_data, last_read_size));
            encoding_data->out_buffer_mutex.unlock();
        }
    }
    
#ifdef WIN32
    if (encoding_data->out_pipe != INVALID_HANDLE_VALUE)
    {
        DisconnectNamedPipe(encoding_data->out_pipe);
    }
#else
    if (encoding_data->out_pipe != 0)
    {
        close(encoding_data->out_pipe);
    }
#endif
}

bool 
get_aud_from_bytestream(std::vector<char> &bytestream, std::vector<hevc_enc_nal_t> &nalus, bool flush, size_t max_data)
{
    unsigned int pos = 0;

    std::vector<int> nalu_start;
    std::vector<int> nalu_end;
    int aud_end;
    bool aud_open = false;
    bool nalu_open = false;

    // first nalu starts right away because we dont want to skip any input bytes
    nalu_start.push_back(pos);
    nalu_open = true;

    // seek AU start
    while (pos < bytestream.size() && aud_open == false)
    {
        if (bytestream[pos + 0] == 0 && bytestream[pos + 1] == 0 && bytestream[pos + 2] == 0 && bytestream[pos + 3] == 1)
        {
            pos += 4; // the size of the aud prefix
            aud_open = true;
        }
        else
        {
            pos += 1;
        }
    }

    if (aud_open == false || pos >= max_data)
    {
        return false;
    }

    // mark NAL start and end positions within the AU
    while (pos < bytestream.size() && aud_open == true)
    {
        if (bytestream[pos + 0] == 0 && bytestream[pos + 1] == 0 && bytestream[pos + 2] == 1 && nalu_open == false)
        {
            nalu_start.push_back(pos);
            pos += 3; // the size of start_code_prefix_one_3bytes
            nalu_open = true;
        }
        else if (bytestream[pos + 0] == 0 && bytestream[pos + 1] == 0 && bytestream[pos + 2] == 0 && bytestream[pos + 3] == 1)
        {
            nalu_end.push_back(pos);
            aud_end = pos;
            nalu_open = false;
            aud_open = false;
        }
        else if (bytestream[pos + 0] == 0 && bytestream[pos + 1] == 0 && bytestream[pos + 2] == 1 && nalu_open == true)
        {
            nalu_end.push_back(pos);
            nalu_open = false;
        }
        else
        {
            pos += 1;
        }
    }
    
    if (flush && aud_open)
    {
        aud_open = false;
        aud_end = pos;
    }
    if (flush && nalu_open)
    {
        nalu_open = false;
        nalu_end.push_back(pos);
    }

    if (aud_open == true || aud_end > max_data)
    {
        return false;
    }

    if (nalu_start.size() != nalu_end.size())
    {
        // ERROR parsing the AU
        return false;
    }

    for (unsigned int nal_idx = 0; nal_idx < nalu_start.size(); nal_idx++)
    {
        hevc_enc_nal_t nal;

        if (nal_idx == 0)
        {
            nal.type = HEVC_ENC_NAL_UNIT_ACCESS_UNIT_DELIMITER;
        }
        else
        {
            nal.type = HEVC_ENC_NAL_UNIT_OTHER;
        }

        nal.size = nalu_end[nal_idx] - nalu_start[nal_idx];
        nal.payload = malloc(nal.size);
        memcpy(nal.payload, bytestream.data() + nalu_start[nal_idx], nal.size);

        nalus.push_back(nal);
    }

    bytestream.erase(bytestream.begin(), bytestream.begin() + aud_end);

    return true;
}

static bool
bin_exists(const std::string& bin)
{
    std::string cmd = bin + " -version";
    int rt = system(cmd.c_str());
    return (rt == 0);
}

bool
parse_init_params
(hevc_enc_ffmpeg_t*              state
,const hevc_enc_init_params_t*   init_params
)
{
    state->data->msg.clear();
    for (unsigned int i = 0; i < init_params->count; i++)
    {
        std::string name(init_params->property[i].name);
        std::string value(init_params->property[i].value);

        if ("bit_depth" == name)
        {
            if (value != "8" && value != "10")
            {
                state->data->msg += "\nInvalid 'bit_depth' value.";
                continue;
            }
            state->data->bit_depth = std::stoi(value);
        }
        else if ("width" == name)
        {
            int width = std::stoi(value);
            if (width < 0)
            {
                state->data->msg += "\nInvalid 'width' value.";
                continue;
            }
            state->data->width = width;
        }
        else if  ("ffmpeg_bin" == name)
        {
            state->data->ffmpeg_bin = value;
        }
        else if ("temp_file" == name)
        {
            state->data->temp_file.push_back(value);
        }
        else if ("height" == name)
        {
            int height = std::stoi(value);
            if (height < 0)
            {
                state->data->msg += "\nInvalid 'height' value.";
                continue;
            }
            state->data->height = height;
        }
        else if ("color_space" == name)
        {
            if (value == "i420")
            {
                state->data->color_space = "yuv420p";
            }
            else if (value == "i422")
            {
                state->data->color_space = "yuv422p";
            }
            else if (value == "i444")
            {
                state->data->color_space = "yuv444p";
            }
            else
            {
                state->data->msg += "\nInvalid 'color_space' value.";
                continue;
            }
        }
        else if ("frame_rate" == name)
        {
            if (value != "23.976"
                &&  value != "24"
                &&  value != "25"
                &&  value != "29.97"
                &&  value != "30"
                &&  value != "59.94"
                &&  value != "60"
                )
            {
                state->data->msg += "\nInvalid 'frame_rate' value.";
                continue;
            }
            state->data->frame_rate = value;
        }
        else if ("absolute_pass_num" == name)
        {
            int pass_num = std::stoi(value);
            if (pass_num < 0)
            {
                state->data->msg += "\nInvalid 'pass_num' value.";
                continue;
            }
            state->data->pass_num = pass_num;
        }
        else if ("data_rate" == name)
        {
            int data_rate = std::stoi(value);
            if (data_rate < 0)
            {
                state->data->msg += "\nInvalid 'data_rate' value.";
                continue;
            }
            state->data->data_rate = data_rate;
        }
        else if ("max_vbv_data_rate" == name)
        {
            int max_vbv_data_rate = std::stoi(value);
            if (max_vbv_data_rate < 0)
            {
                state->data->msg += "\nInvalid 'max_vbv_data_rate' value.";
                continue;
            }
            state->data->max_vbv_data_rate = max_vbv_data_rate;
        }
        else if ("vbv_buffer_size" == name)
        {
            int vbv_buffer_size = std::stoi(value);
            if (vbv_buffer_size < 0)
            {
                state->data->msg += "\nInvalid 'vbv_buffer_size' value.";
                continue;
            }
            state->data->vbv_buffer_size = vbv_buffer_size;
        }
        else if ("command_line" == name)
        {
            state->data->command_line.push_back(value);
        }
    }

    // Following properties are guaranteed to be set by caller,
    // thus must be handled by plugin.    
    if (state->data->command_line.size() <= state->data->pass_num)
    {
        state->data->msg += "\nNot enough ffmpeg command lines provided.";
    }

    if (0 == state->data->bit_depth)
    {
        state->data->msg += "\nMissing 'bit_depth' property.";
    }

    if (0 == state->data->width)
    {
        state->data->msg += "\nMissing 'width' property.";
    }

    if (0 == state->data->height)
    {
        state->data->msg += "\nMissing 'height' property.";
    }

    if (state->data->frame_rate.empty())
    {
        state->data->msg += "\nMissing 'frame_rate' property.";
    }
    
    if (state->data->temp_file.size() < 2)
    {
        state->data->msg += "Need more temp files.";
    }

    if (state->data->ffmpeg_bin.empty())
    {
        state->data->msg += "Path to ffmpeg binary is not set.";
    }
    
    if (!bin_exists(state->data->ffmpeg_bin))
    {
        state->data->msg += "Cannot access ffmpeg binary.";
    }

    return state->data->msg.empty();
}

bool 
create_pipes(hevc_enc_ffmpeg_data_t* data)
{
#ifdef WIN32
    data->temp_file[0] = std::string("\\\\.\\pipe\\") + data->temp_file[0].substr(data->temp_file[0].rfind("\\") + 1);
    data->temp_file[1] = std::string("\\\\.\\pipe\\") + data->temp_file[1].substr(data->temp_file[1].rfind("\\") + 1);

    data->in_pipe = CreateNamedPipe(data->temp_file[0].c_str(),
        PIPE_ACCESS_OUTBOUND,
        PIPE_WAIT | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        1,
        PIPE_BUFFER_SIZE,
        PIPE_BUFFER_SIZE,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);

    data->out_pipe = CreateNamedPipe(data->temp_file[1].c_str(),
        PIPE_ACCESS_INBOUND,
        PIPE_WAIT | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        1,
        PIPE_BUFFER_SIZE,
        PIPE_BUFFER_SIZE,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);

    if (data->in_pipe == NULL || data->out_pipe == NULL)
    {
        return false;
    }
#else
    remove(data->temp_file[0].c_str());
    if (mkfifo(data->temp_file[0].c_str(), S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH) != 0)
    {
        return false;
    }
    remove(data->temp_file[1].c_str());
    if (mkfifo(data->temp_file[1].c_str(), S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH) != 0)
    {
        return false;
    }
#endif

    return true;
}

bool
close_pipes(hevc_enc_ffmpeg_data_t* data)
{
#ifdef WIN32
    CloseHandle(data->in_pipe);
    CloseHandle(data->out_pipe);
#endif
    // no need to close pipes on linux, they are just temp files removed by DEE later

    return true;
}
