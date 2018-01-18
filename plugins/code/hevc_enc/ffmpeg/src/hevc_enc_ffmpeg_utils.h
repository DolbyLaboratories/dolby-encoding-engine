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

#include <vector>
#include <list>
#include <string>
#include <thread>
#include <memory>
#include <mutex>
#include "hevc_enc_api.h"

#ifdef WIN32

#include <windows.h> 
#define PIPE_BUFFER_SIZE 1024 * 1024

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#endif

struct BufferBlob
{
    char*   data;
    size_t  data_size;

    BufferBlob(void* data_to_copy, size_t size);
    ~BufferBlob();
};

typedef struct
{
    std::string                 msg;
    size_t                      max_output_data;
    int                         bit_depth;
    int                         width;
    int                         height;
    std::string                 color_space;
    std::string                 frame_rate;
    std::string                 range;
    int                         pass_num;
    int                         data_rate;
    int                         max_vbv_data_rate;
    int                         vbv_buffer_size;
    std::vector<std::string>    command_line;
    std::vector<char>           output_bytestream;
    std::vector<hevc_enc_nal_t> nalus;

    bool                        light_level_information_sei_present;
    int                         light_level_max_content;
    int                         light_level_max_frame_average;

    bool                        color_description_present;
    std::string                 color_primaries;
    std::string                 transfer_characteristics;
    std::string                 matrix_coefficients;

    bool                        mastering_display_sei_present;
    int                         mastering_display_sei_x1;
    int                         mastering_display_sei_y1;
    int                         mastering_display_sei_x2;
    int                         mastering_display_sei_y2;
    int                         mastering_display_sei_x3;
    int                         mastering_display_sei_y3;
    int                         mastering_display_sei_wx;
    int                         mastering_display_sei_wy;
    int                         mastering_display_sei_max_lum;
    int                         mastering_display_sei_min_lum;

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

    int                         max_pass_num;

    std::string                 multi_pass;
    std::string                 stats_file;

    std::vector<std::string>    temp_file;
    std::string                 ffmpeg_bin;
    std::string                 interpreter;
    std::string                 cmd_gen;
#ifdef WIN32
    HANDLE                      in_pipe;
    HANDLE                      out_pipe;
#else
    int                         in_pipe;
    int                         out_pipe;
#endif
} hevc_enc_ffmpeg_data_t;

typedef struct
{
    hevc_enc_ffmpeg_data_t* data;
    bool                    lib_initialized;
} hevc_enc_ffmpeg_t;

void init_defaults(hevc_enc_ffmpeg_t* state);

bool get_aud_from_bytestream(std::vector<char> &bitstream, std::vector<hevc_enc_nal_t> &nalus, bool flush, size_t max_data);

bool parse_init_params(hevc_enc_ffmpeg_t* state, const hevc_enc_init_params_t* init_params);

void writer_thread_func(hevc_enc_ffmpeg_data_t* data);

void reader_thread_func(hevc_enc_ffmpeg_data_t* data);

void run_cmd_thread_func(std::string cmd, hevc_enc_ffmpeg_data_t* encoding_data);

bool create_pipes(hevc_enc_ffmpeg_data_t* data);

bool close_pipes(hevc_enc_ffmpeg_data_t* data);

bool write_cfg_file(hevc_enc_ffmpeg_data_t* data, const std::string& file);

std::string run_cmd_get_output(std::string cmd);
