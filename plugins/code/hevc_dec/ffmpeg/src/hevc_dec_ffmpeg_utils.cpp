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

#include <fstream>
#include <cstring>
#include <SystemCalls.h>
#include "hevc_dec_ffmpeg_utils.h"

#define BINARY_CHECK_TIMEOUT 1000

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

    int ret_code = 0;
    int status = systemWithTimeout(cmd, ret_code, BINARY_CHECK_TIMEOUT);
    if (status != SYSCALL_STATUS_OK)
    {
        return false;
    }
    else
    {
        return (ret_code == 0);
    }
}

void run_cmd_thread_func(std::string cmd, hevc_dec_ffmpeg_data_t* decoding_data)
{
    int ret_code = 0;
    systemWithKillswitch(cmd, ret_code, decoding_data->kill_ffmpeg);
    decoding_data->ffmpeg_ret_code = ret_code;
    decoding_data->ffmpeg_running = false;
}

void init_picture_buffer(hevc_dec_ffmpeg_data_t* data)
{
    // ONLY YUV 420 8 and 10 bit supported
    int byte_num = data->output_bitdepth > 8 ? 2 : 1;
    data->plane_size[0] = data->width * data->height * byte_num;
    data->plane_size[1] = (data->width * data->height * byte_num) / 4;
    data->plane_size[2] = data->plane_size[1];

    data->decoded_picture.bit_depth = data->output_bitdepth;
    data->decoded_picture.chroma_size = data->plane_size[1];
    data->decoded_picture.luma_size = data->plane_size[0];
    data->decoded_picture.color_space = data->chroma_format;
    data->decoded_picture.frame_rate = data->frame_rate_ext;
    data->decoded_picture.frame_type = HEVC_DEC_FRAME_TYPE_AUTO;
    data->decoded_picture.height = data->height;
    data->decoded_picture.width = data->width;
    data->decoded_picture.matrix_coeffs = 0;
    data->decoded_picture.transfer_characteristics = 0;
    data->decoded_picture.stride[0] = data->width * byte_num;
    data->decoded_picture.stride[1] = (data->width * byte_num) / 2;
    data->decoded_picture.stride[2] = (data->width * byte_num) / 2;
    data->decoded_picture.plane[0] = malloc(data->plane_size[0]);
    data->decoded_picture.plane[1] = malloc(data->plane_size[1]);
    data->decoded_picture.plane[2] = malloc(data->plane_size[2]);
}

void clean_picture_buffer(hevc_dec_ffmpeg_data_t* data)
{
    if (data->decoded_picture.plane[0] != NULL) free(data->decoded_picture.plane[0]);
    if (data->decoded_picture.plane[1] != NULL) free(data->decoded_picture.plane[1]);
    if (data->decoded_picture.plane[2] != NULL) free(data->decoded_picture.plane[2]);
}

hevc_dec_status_t write_to_ffmpeg(hevc_dec_ffmpeg_data_t* data, void* stream_buffer, const size_t buffer_size)
{
    piping_status_t status;

    //printf("!!! writing to ffmpeg !!!\n");
    if (data->encoded_blobs.empty())
    {
        size_t bytes_written = 0;
        status = data->piping_mgr.writeToPipe(data->in_pipe_id, stream_buffer, buffer_size, bytes_written);
        
        if (status != PIPE_MGR_OK)
        {
            data->msg = "Input pipe error " + std::to_string(status) + ".";
            data->piping_error = true;
            return HEVC_DEC_ERROR;
        }
        
        if (bytes_written < buffer_size)
        {
            std::vector<char> new_blob;
            new_blob.assign((char*)stream_buffer + bytes_written, (char*)stream_buffer + buffer_size);
            data->encoded_blobs.push_back(new_blob);
        }

        //printf("!!! blobs left: %d !!!\n", data->encoded_blobs.size());

        return HEVC_DEC_OK;
    }
    else
    {
        std::vector<char> new_blob;
        new_blob.assign((char*)stream_buffer, (char*)stream_buffer + buffer_size);
        data->encoded_blobs.push_back(new_blob);

        size_t bytes_written = 0;
        size_t bytes_to_write = 0;
        do
        {
            std::vector<char> blob_to_write = data->encoded_blobs.front();
            data->encoded_blobs.pop_front();
            bytes_to_write = blob_to_write.size();
            status = data->piping_mgr.writeToPipe(data->in_pipe_id, blob_to_write.data(), bytes_to_write, bytes_written);

            if (status != PIPE_MGR_OK)
            {
                data->msg = "Input pipe error " + std::to_string(status) + ".";
                data->piping_error = true;
                return HEVC_DEC_ERROR;
            }

            if (bytes_written < bytes_to_write)
            {
                //printf("!!! removing from blob: %d, blobs: %d !!!\n", bytes_written, data->encoded_blobs.size());
                blob_to_write.erase(blob_to_write.begin(), blob_to_write.begin() + bytes_written);
                data->encoded_blobs.push_front(blob_to_write);
            }
        } 
        while (bytes_written == bytes_to_write && data->encoded_blobs.empty() == false);

       // printf("!!! blobs left: %d !!!\n", data->encoded_blobs.size());

        return HEVC_DEC_OK;
    }
}

hevc_dec_status_t flush_to_ffmpeg(hevc_dec_ffmpeg_data_t* data)
{
    piping_status_t status;

    //printf("!!! writing to ffmpeg !!!\n");
    if (data->encoded_blobs.empty())
    {
        return HEVC_DEC_PICTURE_NOT_READY;
    }
    else
    {
        size_t bytes_written = 0;
        size_t bytes_to_write = 0;
        do
        {
            std::vector<char> blob_to_write = data->encoded_blobs.front();
            data->encoded_blobs.pop_front();
            bytes_to_write = blob_to_write.size();
            status = data->piping_mgr.writeToPipe(data->in_pipe_id, blob_to_write.data(), bytes_to_write, bytes_written);

            if (status != PIPE_MGR_OK)
            {
                data->msg = "Input pipe error " + std::to_string(status) + ".";
                data->piping_error = true;
                return HEVC_DEC_ERROR;
            }

            if (bytes_written < bytes_to_write)
            {
                //printf("!!! removing from blob: %d, blobs: %d !!!\n", bytes_written, data->encoded_blobs.size());
                blob_to_write.erase(blob_to_write.begin(), blob_to_write.begin() + bytes_written);
                data->encoded_blobs.push_front(blob_to_write);
            }
        } while (bytes_written == bytes_to_write && data->encoded_blobs.empty() == false);

        //printf("!!! blobs left: %d !!!\n", data->encoded_blobs.size());

        return HEVC_DEC_OK;
    }
}

hevc_dec_status_t read_pic_from_ffmpeg(hevc_dec_ffmpeg_data_t* data)
{
    size_t bytes_ready_to_read = 0;
    size_t bytes_read = 0;
    piping_status_t status;
    data->piping_mgr.pipeDataReady(data->out_pipe_id, bytes_ready_to_read);

    if (bytes_ready_to_read >= data->plane_size[0] + data->plane_size[1] + data->plane_size[2])
    {
        for (int i = 0; i < 3; i++)
        {
            status = data->piping_mgr.readFromPipe(data->out_pipe_id, data->decoded_picture.plane[i], data->plane_size[i], bytes_read);
            if ((status != PIPE_MGR_OK && status != PIPE_MGR_PIPE_CLOSED) || bytes_read != data->plane_size[i])
            {
                data->msg = "output pipe error " + std::to_string(status) + ".";
                data->piping_error = true;
                return HEVC_DEC_ERROR;
            }
        }
        return HEVC_DEC_OK;
    }
    else
    {
        return HEVC_DEC_PICTURE_NOT_READY;
    }
}

static void
escape_backslashes(std::string& str)
{
    size_t index = 0;
    for(;;)
    {
        index = str.find("\\", index);
        if (index == std::string::npos) break;

        str.replace(index, 1, "\\\\");

        index += 2;
    }
}

bool
write_cfg_file(hevc_dec_ffmpeg_data_t* data, const std::string& file)
{
    std::ofstream cfg_file(file);
    if (cfg_file.is_open())
    {
        cfg_file << "{\n";
        cfg_file << "    \"plugin_config\": {\n";
        cfg_file << "        \"output_bitdepth\": \""   << data->output_bitdepth                    << "\",\n";
        cfg_file << "        \"width\": \""             << data->width                              << "\",\n";
        cfg_file << "        \"height\": \""            << data->height                             << "\",\n";
        escape_backslashes(data->in_pipe_path);
        cfg_file << "        \"input_file\": \""        << "\\\"" << data->in_pipe_path   << "\\\"" << "\",\n";
        escape_backslashes(data->out_pipe_path);
        cfg_file << "        \"output_file\": \""       << "\\\"" << data->out_pipe_path  << "\\\"" << "\",\n";
        escape_backslashes(data->ffmpeg_bin);
        cfg_file << "        \"ffmpeg_bin\": \""        << "\\\"" << data->ffmpeg_bin     << "\\\"" << "\"\n";
        cfg_file << "    }\n";
        cfg_file << "}\n";
        cfg_file.close();
        return true;
    }
    else
    {
        return false;
    }
}

bool
strip_header(std::string& cmd)
{
    std::string header = "FFMPEG DECODING CMD: ";
    if( header != cmd &&
        header.size() < cmd.size() &&
        cmd.substr(0, header.size()) == header)
    {
        cmd = cmd.substr(header.size(), cmd.size());
        return true;
    }

    return false;
}

void
strip_newline(std::string& str)
{
    size_t index = 0;
    for(;;)
    {
         index = str.find("\n", index);
         if (index == std::string::npos) break;

         str.replace(index, 1, "");
    }
}
