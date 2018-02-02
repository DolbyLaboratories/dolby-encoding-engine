/*
* BSD 3-Clause License
*
* Copyright (c) 2017-2018, Dolby Laboratories
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

#ifndef __DEE_PLUGINS_HEVC_ENC_X265_UTILS_H__
#define __DEE_PLUGINS_HEVC_ENC_X265_UTILS_H__

#include "hevc_enc_api.h"
#include "x265.h"
#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <utility>
#include <fstream>
#include <cstdio>

typedef struct
{
    hevc_enc_nal_type_t type;
    std::vector<char> payload;
} nalu_t;

typedef struct
{
    std::string         msg;
    bool                pending_header;
    bool                info;
    size_t              max_output_data;
    int                 bit_depth;
    int                 width;
    int                 height;
    std::string         profile;
    std::string         color_space;
    std::string         frame_rate;
    std::string         multi_pass;
    std::string         stats_file;
    int                 data_rate;
    int                 max_vbv_data_rate;
    int                 vbv_buffer_size;
    std::string         preset;
    std::string         tune;
    bool                open_gop;
    int                 max_intra_period;
    int                 min_intra_period;
    bool                intra_refresh;
    int                 max_bframes;  
    int                 lookahead_frames;
    int                 frame_threads;
    int                 nr_inter;
    int                 nr_intra;
    int                 cbqpoffs;
    int                 crqpoffs;
    std::string         range;
    int                 scenecut;
    int                 scenecut_bias;
    int                 min_cu_size;
    int                 max_cu_size;
    int                 qg_size;
    bool                rc_grain;

    std::string         color_primaries;
    std::string         transfer_characteristics;
    std::string         matrix_coefficients;

    // master-display
    bool                mastering_display_enabled;
    int                 mastering_display_sei_x1;
    int                 mastering_display_sei_y1;
    int                 mastering_display_sei_x2;
    int                 mastering_display_sei_y2;
    int                 mastering_display_sei_x3;
    int                 mastering_display_sei_y3;
    int                 mastering_display_sei_wx;
    int                 mastering_display_sei_wy;
    int                 mastering_display_sei_max_lum;
    int                 mastering_display_sei_min_lum;

    // max-cll
    bool                light_level_enabled;
    int                 light_level_max_content;
    int                 light_level_max_frame_average;

    std::string         level_idc;
    std::string         psy_rd;
    std::vector<nalu_t>                             output_buffer;
    std::vector<nalu_t>::iterator                   last_used_nal;
    std::vector<hevc_enc_nal_t>                     output;
    std::list<std::pair<std::string,std::string>>   internal_params;
} hevc_enc_x265_data_t;

/* This structure can contain only pointers and simple types */
typedef struct
{
    
    const x265_api*         api;
    x265_param*             param;
    x265_encoder*           encoder;
    hevc_enc_x265_data_t*   data;
    bool                    lib_initialized;
} hevc_enc_x265_t;

void
init_defaults
    (hevc_enc_x265_t* state);

bool
parse_init_params
    (hevc_enc_x265_t*               state
    ,const hevc_enc_init_params_t*  init_params);

bool
set_param
    (hevc_enc_x265_t*   state
    ,const std::string& name
    ,const std::string& value);

bool
set_preset
    (hevc_enc_x265_t*   state
    ,const std::string& preset
    ,const std::string& tune);

hevc_enc_nal_type_t
cast_nal_type
    (const uint32_t type);

int
get_color_prim_number
    (std::string);

int
get_transfer_characteristics_number
    (std::string);

int
get_matrix_coefficients_number
    (std::string);

void
get_config_msg
    (hevc_enc_x265_t* state
    ,std::list<std::pair<std::string,std::string>>& native_params
    ,std::string&     msg);

int
frametype_to_slicetype
    (frame_type_t in_type);

bool
filter_native_params
    (hevc_enc_x265_t* state
    ,std::list<std::pair<std::string,std::string>>& params);

#endif // __DEE_PLUGINS_HEVC_ENC_X265_UTILS_H__
