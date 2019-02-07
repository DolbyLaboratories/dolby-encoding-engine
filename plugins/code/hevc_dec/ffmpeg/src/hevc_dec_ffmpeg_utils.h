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

#include <string>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include "hevc_dec_api.h"
#include "PipingManager.h"
#include "GenericPlugin.h"

typedef struct
{
    int                         output_bitdepth;
    int                         width;
    int                         height;
    int                         byte_num;
    HevcDecColorSpace           chroma_format;
    HevcDecFrameRate            frame_rate;
    HevcDecFrameRate            frame_rate_ext;
    std::string                 output_format;
    int                         transfer_characteristics;
    int                         matrix_coeffs;
    std::string                 msg;

    HevcDecPicture              decoded_picture;
    size_t                      plane_size[3];

    std::thread                 ffmpeg_thread;
    int                         ffmpeg_ret_code;

    std::vector<std::string>    temp_file;
    std::string                 ffmpeg_bin;
    std::string                 interpreter;
    std::string                 cmd_gen;
    std::atomic_bool            kill_ffmpeg;
    std::atomic_bool            ffmpeg_running;

    std::string                 in_pipe_path;
    std::string                 out_pipe_path;
    PipingManager               piping_mgr;
    int                         in_pipe_id;
    int                         out_pipe_id;
    bool                        piping_error;
    bool                        redirect_stdout;

    GenericPlugin               generic_plugin;

} hevc_dec_ffmpeg_data_t;

std::string print_ffmpeg_state(hevc_dec_ffmpeg_data_t* data);

void init_picture_buffer(hevc_dec_ffmpeg_data_t* data);

void clean_picture_buffer(hevc_dec_ffmpeg_data_t* data);

HevcDecFrameRate string_to_fr(const std::string& str);

bool bin_exists(const std::string& bin, const std::string& arg, std::string& output);

void run_cmd_thread_func(std::string cmd, hevc_dec_ffmpeg_data_t* decoding_data);

HevcDecStatus read_pic_from_ffmpeg(hevc_dec_ffmpeg_data_t* data);

bool strip_header(std::string& cmd);

void strip_newline(std::string& str);

bool write_cfg_file(hevc_dec_ffmpeg_data_t* data, const std::string& file);
