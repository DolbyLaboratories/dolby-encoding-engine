/*
* BSD 3-Clause License
*
* Copyright (c) 2017-2019, Dolby Laboratories
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
#include <atomic>
#include "SystemCalls.h"
#include "PipingManager.h"
#include "GenericPlugin.h"
#include "hevc_enc_api.h"

#define READ_BUFFER_SIZE 1024

#include <iostream>

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
    std::vector<HevcEncNal>     nalus;
    char                        output_temp_buf[READ_BUFFER_SIZE];

    bool                        light_level_information_sei_present;
    int                         light_level_max_content;
    int                         light_level_max_frame_average;

    bool                        color_description_present;
    std::string                 color_primaries;
    std::string                 transfer_characteristics;
    std::string                 matrix_coefficients;
    std::string                 chromaSampleLocation;

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

    std::thread                 ffmpeg_thread;
    int                         ffmpeg_ret_code;

    int                         max_pass_num;

    std::string                 multi_pass;
    std::string                 stats_file;

    std::vector<std::string>    temp_file;
    std::string                 ffmpeg_bin;
    std::string                 interpreter;
    std::string                 cmd_gen;
    std::string                 user_params_file;
    std::atomic_bool            kill_ffmpeg;
    std::atomic_bool            ffmpeg_running;

    std::string                 in_pipe_path;
    std::string                 out_pipe_path;
    PipingManager               piping_mgr;
    int                         in_pipe_id;
    int                         out_pipe_id;
    bool                        piping_error;
    bool                        redirect_stdout;

    std::string                 version_string;

    GenericPlugin               generic_plugin;

} hevc_enc_ffmpeg_data_t;

typedef struct
{
    hevc_enc_ffmpeg_data_t* data;
    bool                    lib_initialized;
} hevc_enc_ffmpeg_t;

void init_defaults(hevc_enc_ffmpeg_t* state);

bool get_aud_from_bytestream(std::vector<char> &bitstream, std::vector<HevcEncNal> &nalus, bool flush, size_t max_data);

bool parse_init_params(hevc_enc_ffmpeg_t* state, const HevcEncInitParams* init_params);

void run_cmd_thread_func(std::string cmd, hevc_enc_ffmpeg_data_t* encoding_data);

bool write_cfg_file(hevc_enc_ffmpeg_data_t* data, const std::string& file);

void clear_nalu_buffer(hevc_enc_ffmpeg_data_t* data);

std::string fps_to_num_denom(const std::string& fps);

bool strip_header(std::string& cmd);

void strip_newline(std::string& str);
