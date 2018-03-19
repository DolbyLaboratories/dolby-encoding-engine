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

#include <string>
#include <vector>
#include <deque>
#include <list>
#include <thread>
#include <memory>
#include <mutex>
#include "hevc_dec_api.h"
#include "NamedPipe.h"

#define READ_BUFFER_SIZE 1024 * 1024

struct BufferBlob
{
    char*   data;
    size_t  data_size;

    BufferBlob(const void* data_to_copy, size_t size);
    ~BufferBlob();
};

typedef struct
{
    int                         output_bitdepth;
    int                         width;
    int                         height;
    hevc_dec_color_space_t      chroma_format;
    hevc_dec_frame_rate_t       frame_rate;
    hevc_dec_frame_rate_t       frame_rate_ext;
    std::string                 output_format;
    int                         transfer_characteristics;
    int                         matrix_coeffs;
    std::string                 msg;

    std::deque<hevc_dec_picture_t> decoded_pictures;
    size_t                      decoded_pictures_to_discard;

    std::thread                 writer_thread;
    std::thread                 reader_thread;
    std::thread                 ffmpeg_thread;
    std::list<BufferBlob*>      in_buffer;
    std::list<BufferBlob*>      out_buffer;
    std::mutex                  in_buffer_mutex;
    std::mutex                  out_buffer_mutex;
    bool                        stop_writing_thread;
    bool                        stop_reading_thread;
    bool                        force_stop_writing_thread;
    bool                        force_stop_reading_thread;
    int                         ffmpeg_ret_code;

    std::vector<std::string>    temp_file;
    std::string                 ffmpeg_bin;
    std::string                 interpreter;
    std::string                 cmd_gen;

    NamedPipe                   in_pipe;
    NamedPipe                   out_pipe;

    // Benchamrk
    unsigned long long missed_calls;
    unsigned long long calls;

} hevc_dec_ffmpeg_data_t;

void remove_pictures(hevc_dec_ffmpeg_data_t* data);

hevc_dec_frame_rate_t string_to_fr(const std::string& str);

bool bin_exists(const std::string& bin, const std::string& arg);

void run_cmd_thread_func(std::string cmd, hevc_dec_ffmpeg_data_t* decoding_data);

void writer_thread_func(hevc_dec_ffmpeg_data_t* decoding_data);

void reader_thread_func(hevc_dec_ffmpeg_data_t* decoding_data);

int extract_bytes(std::list<BufferBlob*>& blob_list, size_t byte_num, char* buffer);

size_t get_buf_size(std::list<BufferBlob*>& blob_list);

int extract_pictures_from_buffer(hevc_dec_ffmpeg_data_t* data);

bool write_cfg_file(hevc_dec_ffmpeg_data_t* data, const std::string& file);

std::string run_cmd_get_output(std::string cmd);