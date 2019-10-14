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

#include "hevc_enc_x265_utils.h"
#include <map>
#include <algorithm>
#include <sstream>
#include <iterator>

static
std::map<std::string, int> color_primaries_map = {
    { "bt_709", 1 },
    { "unspecified", 2 },
    { "bt_601_625", 5 },
    { "bt_601_525", 6 },
    { "bt_2020", 9 },
};

static
std::map<std::string, int> transfer_characteristics_map = {
    { "bt_709", 1 },
    { "unspecified", 2 },
    { "bt_601_625", 4 },
    { "bt_601_525", 6 },
    { "smpte_st_2084", 16 },
    { "std_b67", 18 },
};

static
std::map<std::string, int> matrix_coefficients_map = {
    { "bt_709", 1 },
    { "unspecified", 2 },
    { "bt_601_625", 4 },
    { "bt_601_525", 6 },
    { "bt_2020", 9 },
};

template<typename T>
static
void 
split_string
    (const std::string &s
    ,char delimiter
    ,T result) 
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delimiter)) 
    {
        *(result++) = item;
    }
}

static
std::vector<std::string>
split_string
    (const std::string &s
    ,char delimiter)
{
    std::vector<std::string> items;
    split_string(s, delimiter, std::back_inserter(items));
    return items;
}

int
get_color_prim_number(std::string color_prim)
{
    auto it = color_primaries_map.find(color_prim);
    if (it == color_primaries_map.end())
    {
        return -1;
    }
    return it->second;
}

int
get_transfer_characteristics_number(std::string transfer)
{
    auto it = transfer_characteristics_map.find(transfer);
    if (it == transfer_characteristics_map.end())
    {
        return -1;
    }
    return it->second;
}

int
get_matrix_coefficients_number(std::string matrix)
{
    auto it = matrix_coefficients_map.find(matrix);
    if (it == matrix_coefficients_map.end())
    {
        return -1;
    }
    return it->second;
}

int
frametype_to_slicetype(HevcEncFrameType in_type)
{
    int ret_val;

    switch (in_type)
    {
    case HEVC_ENC_FRAME_TYPE_IDR:
        ret_val = X265_TYPE_IDR;
        break;
    case HEVC_ENC_FRAME_TYPE_I:
        ret_val = X265_TYPE_I;
        break;
    case HEVC_ENC_FRAME_TYPE_P:
        ret_val = X265_TYPE_P;
        break;
    case HEVC_ENC_FRAME_TYPE_B:
        ret_val = X265_TYPE_B;
        break;
    case HEVC_ENC_FRAME_TYPE_BREF:
        ret_val = X265_TYPE_BREF;
        break;
    default:
    case HEVC_ENC_FRAME_TYPE_AUTO:
        ret_val = X265_TYPE_AUTO;
        break;
    }

    return ret_val;
}

void
init_defaults(hevc_enc_x265_t* state)
{
    state->data->bit_depth = 0;         /**< Must be set by caller */
    state->data->width = 0;             /**< Must be set by caller */
    state->data->height = 0;            /**< Must be set by caller */
    state->data->frame_rate.clear();    /**< Must be set by caller */
    state->data->color_space = "i420";       
    state->data->multi_pass = "off";
    state->data->data_rate = 15000;
    state->data->max_vbv_data_rate = 15000;
    state->data->vbv_buffer_size = 30000;
    state->data->preset = "medium";
    state->data->tune = "none";
    state->data->profile = "auto";
    state->data->open_gop = false;
    state->data->max_intra_period = 250;
    state->data->min_intra_period = 0;
    state->data->intra_refresh = false;
    state->data->max_bframes = 4;
    state->data->lookahead_frames = 20;
    state->data->max_output_data = 0;
    state->data->range = "full";
    state->data->info = false;
    state->data->frame_threads = 0;
    state->data->nr_inter = 0;
    state->data->nr_intra = 0;
    state->data->cbqpoffs = 0;
    state->data->crqpoffs = 0;
    state->data->scenecut = 40;
    state->data->scenecut_bias = 5;
    state->data->min_cu_size = 8;
    state->data->max_cu_size = 64;
    state->data->qg_size = 64;
    state->data->rc_grain = false;
    state->data->level_idc = "0";
    state->data->psy_rd = "2.0";
    state->data->wpp = false;

    state->data->color_primaries = "unspecified";
    state->data->transfer_characteristics = "unspecified";
    state->data->matrix_coefficients = "unspecified";
    state->data->chromaSampleLocation = "0";

    // master-display
    state->data->mastering_display_enabled = false;
    state->data->mastering_display_sei_x1 = 0;
    state->data->mastering_display_sei_y1 = 0;
    state->data->mastering_display_sei_x2 = 0;
    state->data->mastering_display_sei_y2 = 0;
    state->data->mastering_display_sei_x3 = 0;
    state->data->mastering_display_sei_y3 = 0;
    state->data->mastering_display_sei_wx = 0;
    state->data->mastering_display_sei_wy = 0;
    state->data->mastering_display_sei_max_lum = 0;
    state->data->mastering_display_sei_min_lum = 0;

    // max-cll
    state->data->light_level_enabled = false;
    state->data->light_level_max_content = 0;
    state->data->light_level_max_frame_average = 0;

    state->data->force_slice_type = false;
    state->data->uhd_bd = false;
    state->data->concatenation_flag = false;

    state->data->plugin_path.clear();
    state->data->config_path.clear();
}

bool
parse_init_params
    (hevc_enc_x265_t*               state
    ,const HevcEncInitParams*  init_params
    )
{
    state->data->msg.clear();
    for (size_t i = 0; i < init_params->count; i++)
    {
        std::string name(init_params->properties[i].name);
        std::string value(init_params->properties[i].value);

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
            if (value != "i400" 
                && value != "i420"
                && value != "i422"
                && value != "i444"
                )
            {
                state->data->msg += "\nInvalid 'color_space' value.";
                continue;
            }
            state->data->color_space = value;
        }
        else if ("frame_rate" == name)
        {
            if  (   value != "23.976"
                &&  value != "24"
                &&  value != "25"
                &&  value != "29.97"
                &&  value != "30"
                &&  value != "48"
                &&  value != "50"
                &&  value != "59.94"
                &&  value != "60"
                )
            {
                state->data->msg += "\nInvalid 'frame_rate' value.";
                continue;
            }
            state->data->frame_rate = value;
        }
        else if ("frame_threads" == name)
        {
            int threads = std::stoi(value);
            if (threads < 0 || threads > 16)
            {
                state->data->msg += "\nInvalid 'frame_threads' value.";
                continue;
            }
            state->data->frame_threads = threads;
        }
        else if ("nr_inter" == name)
        {
            int nr_inter = std::stoi(value);
            if (nr_inter < 0 || nr_inter > 2000)
            {
                state->data->msg += "\nInvalid 'nr_inter' value.";
                continue;
            }
            state->data->nr_inter = nr_inter;
        }
        else if ("nr_intra" == name)
        {
            int nr_intra = std::stoi(value);
            if (nr_intra < 0 || nr_intra > 2000)
            {
                state->data->msg += "\nInvalid 'nr_intra' value.";
                continue;
            }
            state->data->nr_intra = nr_intra;
        }
        else if ("crqpoffs" == name)
        {
            int crqpoffs = std::stoi(value);
            if (crqpoffs < -12 || crqpoffs > 12)
            {
                state->data->msg += "\nInvalid 'crqpoffs' value.";
                continue;
            }
            state->data->crqpoffs = crqpoffs;
        }
        else if ("cbqpoffs" == name)
        {
            int cbqpoffs = std::stoi(value);
            if (cbqpoffs < -12 || cbqpoffs > 12)
            {
                state->data->msg += "\nInvalid 'cbqpoffs' value.";
                continue;
            }
            state->data->cbqpoffs = cbqpoffs;
        }
        else if ("range" == name)
        {
            if (value != "limited" && value != "full")
            {
                state->data->msg += "\nInvalid 'range' value.";
                continue;
            }
            state->data->range = value;
        }
        else if ("profile" == name)
        {
            if (value != "auto"
                && value != "main"
                && value != "main10"
                && value != "main-intra"
                && value != "main10-intra"
                && value != "main444-8"
                && value != "main444-intra"
                && value != "main422-10"
                && value != "main422-10-intra"
                && value != "main444-10"
                && value != "main444-10-intra"
                && value != "main12"
                && value != "main12-intra"
                && value != "main422-12"
                && value != "main422-12-intra"
                && value != "main444-12"
                && value != "main444-12-intra"
                )
            {
                state->data->msg += "\nInvalid 'profile' value.";
                continue;
            }
            state->data->profile = value;
        }
        else if ("multi_pass" == name)
        {
            if (value != "off" 
                && value != "1st"
                && value != "nth"
                && value != "last"
                )
            {
                state->data->msg += "\nInvalid 'multi_pass' value.";
                continue;
            }
            state->data->multi_pass = value;
        }
        else if ("stats_file" == name)
        {
            state->data->stats_file = value;
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
        else if ("preset" == name)
        {
            if (value != "ultrafast" 
                && value != "superfast"
                && value != "veryfast"
                && value != "faster"
                && value != "fast"
                && value != "medium"
                && value != "slow"
                && value != "slower"
                && value != "veryslow"
                && value != "placebo"
                )
            {
                state->data->msg += "\nInvalid 'preset' value.";
                continue;
            }
            state->data->preset = value;
        }
        else if ("tune" == name)
        {
            if (value != "none" 
                && value != "psnr"
                && value != "ssim"
                && value != "grain"
                && value != "fastdecode"
                && value != "zerolatency"
                )
            {
                state->data->msg += "\nInvalid 'tune' value.";
                continue;
            }
            state->data->tune = value;
        }
        else if ("open_gop" == name)
        {
            if (value != "true" && value != "false")
            {
                state->data->msg += "\nInvalid 'open_gop' value.";
                continue;
            }
            state->data->open_gop = (value == "true");
        }
        else if ("max_intra_period" == name)
        {
            int max_intra_period = std::stoi(value);
            if (max_intra_period < -1)
            {
                state->data->msg += "\nInvalid 'max_intra_period' value.";
                continue;
            }
            state->data->max_intra_period = max_intra_period;
        }
        else if ("min_intra_period" == name)
        {
            int min_intra_period = std::stoi(value);
            if (min_intra_period < 0)
            {
                state->data->msg += "\nInvalid 'min_intra_period' value.";
                continue;
            }
            state->data->min_intra_period = min_intra_period;
        }
        else if ("intra_refresh" == name)
        {
            if (value != "true" && value != "false")
            {
                state->data->msg += "\nInvalid 'intra_refresh' value.";
                continue;
            }
            state->data->intra_refresh = (value == "true");
        }
        else if ("max_bframes" == name)
        {
            int max_bframes = std::stoi(value);
            if (max_bframes < 0 || max_bframes > 16)
            {
                state->data->msg += "\nInvalid 'max_bframes' value.";
                continue;
            }
            state->data->max_bframes = max_bframes;
        }
        else if ("lookahead_frames" == name)
        {
            int lookahead_frames = std::stoi(value);
            if (lookahead_frames < 0 || lookahead_frames > 250)
            {
                state->data->msg += "\nInvalid 'lookahead_frames' value.";
                continue;
            }
            state->data->lookahead_frames = lookahead_frames;
        }
        else if ("info" == name)
        {
            if (value != "true" && value != "false")
            {
                state->data->msg += "\nInvalid 'info' value.";
                continue;
            }
            state->data->info = (value == "true");
        }
        else if ("scenecut" == name)
        {
            int scenecut = std::stoi(value);
            if (scenecut < 0)
            {
                state->data->msg += "\nInvalid 'scenecut' value.";
                continue;
            }
            state->data->scenecut = scenecut;
        }
        else if ("scenecut_bias" == name)
        {
            int scenecut_bias = std::stoi(value);
            if (scenecut_bias < 0 || scenecut_bias > 100)
            {
                state->data->msg += "\nInvalid 'scenecut_bias' value.";
                continue;
            }
            state->data->scenecut_bias = scenecut_bias;
        }
        else if ("min_cu_size" == name)
        {
            int min_cu_size = std::stoi(value);
            if (min_cu_size < 8 || min_cu_size > 32)
            {
                state->data->msg += "\nInvalid 'min_cu_size' value.";
                continue;
            }
            state->data->min_cu_size = min_cu_size;
        }
        else if ("max_cu_size" == name)
        {
            int max_cu_size = std::stoi(value);
            if (max_cu_size < 16 || max_cu_size > 64)
            {
                state->data->msg += "\nInvalid 'max_cu_size' value.";
                continue;
            }
            state->data->max_cu_size = max_cu_size;
        }
        else if ("qg_size" == name)
        {
            int qg_size = std::stoi(value);
            if (qg_size < 16 || qg_size > 64)
            {
                state->data->msg += "\nInvalid 'qg_size' value.";
                continue;
            }
            state->data->qg_size = qg_size;
        }
        else if ("rc_grain" == name)
        {
            if (value != "true" && value != "false")
            {
                state->data->msg += "\nInvalid 'rc_grain' value.";
                continue;
            }
            state->data->rc_grain = (value == "true");
        }
        else if ("level_idc" == name)
        {
            if (value != "0"
                && value != "1.0"
                && value != "10"
                && value != "2.0"
                && value != "20"
                && value != "2.1"
                && value != "21"
                && value != "3.0"
                && value != "30"
                && value != "3.1"
                && value != "31"
                && value != "4.0"
                && value != "40"
                && value != "4.1"
                && value != "41"
                && value != "5.0"
                && value != "50"
                && value != "5.1"
                && value != "51"
                && value != "5.2"
                && value != "52"
                && value != "6.0"
                && value != "60"
                && value != "6.1"
                && value != "61"
                && value != "6.2"
                && value != "62"
                && value != "8.5"
                && value != "85"
                )
            {
                state->data->msg += "\nInvalid 'level_idc' value.";
                continue;
            }
            state->data->level_idc = value;
        }
        else if ("psy_rd" == name)
        {
            float psy_rd_float = std::stof(value);
            if (psy_rd_float < 0 || psy_rd_float > 5)
            {
                state->data->msg += "\nInvalid 'psy_rd' value.";
                continue;
            }
            state->data->psy_rd = value;
        }
        else if ("wpp" == name) {
            if (value != "true" && value != "false")
            {
                state->data->msg += "\nInvalid 'wpp' value.";
                continue;
            }
            state->data->wpp = (value == "true");
        }
        else if ("color_primaries" == name)
        {
            auto it = color_primaries_map.find(value);
            if (it == color_primaries_map.end())
            {
                state->data->msg += "\nInvalid 'color_primaries' value.";
                continue;
            }
            state->data->color_primaries = value;
        }
        else if ("transfer_characteristics" == name)
        {
            auto it = transfer_characteristics_map.find(value);
            if (it == transfer_characteristics_map.end())
            {
                state->data->msg += "\nInvalid 'transfer_characteristics' value.";
                continue;
            }
            state->data->transfer_characteristics = value;
        }
        else if ("matrix_coefficients" == name)
        {
            auto it = matrix_coefficients_map.find(value);
            if (it == matrix_coefficients_map.end())
            {
                state->data->msg += "\nInvalid 'matrix_coefficients' value.";
                continue;
            }
            state->data->matrix_coefficients = value;
        }
        else if ("chromaloc" == name)
        {
            state->data->chromaSampleLocation = value;
        }
        // master-display
        else if ("mastering_display_sei_x1" == name)
        {
            state->data->mastering_display_enabled = true;
            int mastering_display_sei_x1 = std::stoi(value);
            if (mastering_display_sei_x1 < 0 || mastering_display_sei_x1 > 50000)
            {
                state->data->msg += "\nInvalid 'mastering_display_sei_x1' value.";
                continue;
            }
            state->data->mastering_display_sei_x1 = mastering_display_sei_x1;
        }
        else if ("mastering_display_sei_y1" == name)
        {
            state->data->mastering_display_enabled = true;
            int mastering_display_sei_y1 = std::stoi(value);
            if (mastering_display_sei_y1 < 0 || mastering_display_sei_y1 > 50000)
            {
                state->data->msg += "\nInvalid 'mastering_display_sei_y1' value.";
                continue;
            }
            state->data->mastering_display_sei_y1 = mastering_display_sei_y1;
        }
        else if ("mastering_display_sei_x2" == name)
        {
            state->data->mastering_display_enabled = true;
            int mastering_display_sei_x2 = std::stoi(value);
            if (mastering_display_sei_x2 < 0 || mastering_display_sei_x2 > 50000)
            {
                state->data->msg += "\nInvalid 'mastering_display_sei_x2' value.";
                continue;
            }
            state->data->mastering_display_sei_x2 = mastering_display_sei_x2;
        }
        else if ("mastering_display_sei_y2" == name)
        {
            state->data->mastering_display_enabled = true;
            int mastering_display_sei_y2 = std::stoi(value);
            if (mastering_display_sei_y2 < 0 || mastering_display_sei_y2 > 50000)
            {
                state->data->msg += "\nInvalid 'mastering_display_sei_y2' value.";
                continue;
            }
            state->data->mastering_display_sei_y2 = mastering_display_sei_y2;
        }
        else if ("mastering_display_sei_x3" == name)
        {
            state->data->mastering_display_enabled = true;
            int mastering_display_sei_x3 = std::stoi(value);
            if (mastering_display_sei_x3 < 0 || mastering_display_sei_x3 > 50000)
            {
                state->data->msg += "\nInvalid 'mastering_display_sei_x3' value.";
                continue;
            }
            state->data->mastering_display_sei_x3 = mastering_display_sei_x3;
        }
        else if ("mastering_display_sei_y3" == name)
        {
            state->data->mastering_display_enabled = true;
            int mastering_display_sei_y3 = std::stoi(value);
            if (mastering_display_sei_y3 < 0 || mastering_display_sei_y3 > 50000)
            {
                state->data->msg += "\nInvalid 'mastering_display_sei_y3' value.";
                continue;
            }
            state->data->mastering_display_sei_y3 = mastering_display_sei_y3;
        }
        else if ("mastering_display_sei_wx" == name)
        {
            state->data->mastering_display_enabled = true;
            int mastering_display_sei_wx = std::stoi(value);
            if (mastering_display_sei_wx < 0 || mastering_display_sei_wx > 50000)
            {
                state->data->msg += "\nInvalid 'mastering_display_sei_wx' value.";
                continue;
            }
            state->data->mastering_display_sei_wx = mastering_display_sei_wx;
        }
        else if ("mastering_display_sei_wy" == name)
        {
            state->data->mastering_display_enabled = true;
            int mastering_display_sei_wy = std::stoi(value);
            if (mastering_display_sei_wy < 0 || mastering_display_sei_wy > 50000)
            {
                state->data->msg += "\nInvalid 'mastering_display_sei_wy' value.";
                continue;
            }
            state->data->mastering_display_sei_wy = mastering_display_sei_wy;
        }
        else if ("mastering_display_sei_max_lum" == name)
        {
            state->data->mastering_display_enabled = true;
            int mastering_display_sei_max_lum = std::stoi(value);
            if (mastering_display_sei_max_lum < 0 || mastering_display_sei_max_lum > 2000000000)
            {
                state->data->msg += "\nInvalid 'mastering_display_sei_max_lum' value.";
                continue;
            }
            state->data->mastering_display_sei_max_lum = mastering_display_sei_max_lum;
        }
        else if ("mastering_display_sei_min_lum" == name)
        {
            state->data->mastering_display_enabled = true;
            int mastering_display_sei_min_lum = std::stoi(value);
            if (mastering_display_sei_min_lum < 0 || mastering_display_sei_min_lum > 2000000000)
            {
                state->data->msg += "\nInvalid 'mastering_display_sei_min_lum' value.";
                continue;
            }
            state->data->mastering_display_sei_min_lum = mastering_display_sei_min_lum;
        }
        // max-cll
        else if ("light_level_max_content" == name)
        {
            state->data->light_level_enabled = true;
            int light_level_max_content = std::stoi(value);
            if (light_level_max_content < 0 || light_level_max_content > 65535)
            {
                state->data->msg += "\nInvalid 'light_level_max_content' value.";
                continue;
            }
            state->data->light_level_max_content = light_level_max_content;
        }
        else if ("light_level_max_frame_average" == name)
        {
            state->data->light_level_enabled = true;
            int light_level_max_frame_average = std::stoi(value);
            if (light_level_max_frame_average < 0 || light_level_max_frame_average > 65535)
            {
                state->data->msg += "\nInvalid 'light_level_max_frame_average' value.";
                continue;
            }
            state->data->light_level_max_frame_average = light_level_max_frame_average;
        }
        else if ("force_slice_type" == name)
        {
            state->data->force_slice_type = ("true" == value);
        }
        else if ("uhd_bd" == name)
        {
            state->data->uhd_bd = ("true" == value);
        }
        else if ("concatenation_flag" == name)
        {
            state->data->concatenation_flag = ("true" == value);
        }
        else if ("absolute_pass_num" == name)
        {
            // this internal param is ignored for x265
            continue;
        }
        else if ("param" == name)
        {
            auto items = split_string(value, ':');
            for (auto item : items)
            {  
                std::string param_name, param_value;
                std::size_t found = item.find("=");
                if (found == std::string::npos)
                {
                    param_name = item;
                }
                else
                {
                    param_name = item.substr(0, found);
                    param_value = item.substr(found+1);
                    std::replace(param_value.begin(), param_value.end(), ';', ':');
                }
                state->data->internal_params.push_back({param_name, param_value});
            }
        }
        else if ("plugin_path" == name)
        {
            state->data->plugin_path = value;
        }
        else if ("config_path" == name)
        {
            state->data->config_path = value;
        }
        else
        {
            state->data->msg += "\nUnknown XML property: " + name;
        }
    }

    // Following properties are guaranteed to be set by caller,
    // thus must be handled by plugin.
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

    return state->data->msg.empty();
}

bool
set_param
    (hevc_enc_x265_t*   state
    ,const std::string& name
    ,const std::string& value)
{
    int status;
    if (name.empty()) return false;
    if (value.empty()) status = state->api->param_parse(state->param, name.c_str(), NULL);
    else status = state->api->param_parse(state->param, name.c_str(), value.c_str());
    if(status != 0) state->data->msg += "\nCould not set '" + name + "' to '" + value +"'.";
    return (status == 0);
}

bool
set_preset
    (hevc_enc_x265_t*   state
    ,const std::string& preset
    ,const std::string& tune)
{
    int status;
    if (preset.empty()) return false;
    if (tune.empty() || tune == "none") status = state->api->param_default_preset(state->param, preset.c_str(), NULL);
    else status = state->api->param_default_preset(state->param, preset.c_str(), tune.c_str());
    if(status != 0) state->data->msg += "\nCould not set preset '" + preset + "' and  tune '" + tune +"'.";
    return (status == 0);
}

HevcEncNalType
cast_nal_type(const uint32_t type)
{
    HevcEncNalType out;
    if (type == NAL_UNIT_CODED_SLICE_TRAIL_N) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_TRAIL_N;
    else if (type == NAL_UNIT_CODED_SLICE_TRAIL_R) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_TRAIL_R;
    else if (type == NAL_UNIT_CODED_SLICE_TSA_N) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_TSA_N;
    else if (type == NAL_UNIT_CODED_SLICE_TLA_R) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_TLA_R;
    else if (type == NAL_UNIT_CODED_SLICE_STSA_N) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_STSA_N;
    else if (type == NAL_UNIT_CODED_SLICE_STSA_R) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_STSA_R;
    else if (type == NAL_UNIT_CODED_SLICE_RADL_R) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_RADL_N;
    else if (type == NAL_UNIT_CODED_SLICE_RASL_N) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_RADL_R;
    else if (type == NAL_UNIT_CODED_SLICE_RASL_R) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_RASL_R;
    else if (type == NAL_UNIT_CODED_SLICE_BLA_W_LP) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_BLA_W_LP;
    else if (type == NAL_UNIT_CODED_SLICE_BLA_W_RADL) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_BLA_W_RADL;
    else if (type == NAL_UNIT_CODED_SLICE_BLA_N_LP) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_BLA_N_LP;
    else if (type == NAL_UNIT_CODED_SLICE_IDR_W_RADL) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_IDR_W_RADL;
    else if (type == NAL_UNIT_CODED_SLICE_IDR_N_LP) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_IDR_N_LP;
    else if (type == NAL_UNIT_CODED_SLICE_CRA) out = HEVC_ENC_NAL_UNIT_CODED_SLICE_CRA;
    else if (type == NAL_UNIT_VPS) out = HEVC_ENC_NAL_UNIT_VPS;
    else if (type == NAL_UNIT_SPS) out = HEVC_ENC_NAL_UNIT_SPS;
    else if (type == NAL_UNIT_PPS) out = HEVC_ENC_NAL_UNIT_PPS;
    else if (type == NAL_UNIT_ACCESS_UNIT_DELIMITER) out = HEVC_ENC_NAL_UNIT_ACCESS_UNIT_DELIMITER;
    else if (type == NAL_UNIT_EOS) out = HEVC_ENC_NAL_UNIT_EOS;
    else if (type == NAL_UNIT_EOB) out = HEVC_ENC_NAL_UNIT_EOB;
    else if (type == NAL_UNIT_FILLER_DATA) out = HEVC_ENC_NAL_UNIT_FILLER_DATA;
    else if (type == NAL_UNIT_PREFIX_SEI) out = HEVC_ENC_NAL_UNIT_PREFIX_SEI;
    else if (type == NAL_UNIT_SUFFIX_SEI) out = HEVC_ENC_NAL_UNIT_SUFFIX_SEI;
    else if (type == NAL_UNIT_INVALID) out = HEVC_ENC_NAL_UNIT_OTHER;
    else  out = HEVC_ENC_NAL_UNIT_OTHER;
    
    return out;
}

static
std::string
bool2string(const bool val)
{
    return val ? "true" : "false";
}

void
get_config_msg
    (hevc_enc_x265_t* state
    ,std::list<std::pair<std::string,std::string>>& native_params
    ,std::string&     msg)
{
    msg = "x265 encoder version: " + std::string(x265_version_str);
    msg += "\nx265 plugin configuration:";
    msg += "\n  bit_depth="+std::to_string(state->data->bit_depth);
    msg += "\n  width="+std::to_string(state->data->width);
    msg += "\n  height="+std::to_string(state->data->height);
    msg += "\n  color_space="+state->data->color_space;
    msg += "\n  frame_rate="+state->data->frame_rate;
    msg += "\n  data_rate="+std::to_string(state->data->data_rate);
    msg += "\n  max_vbv_data_rate="+std::to_string(state->data->max_vbv_data_rate);
    msg += "\n  vbv_buffer_size="+std::to_string(state->data->vbv_buffer_size);
    msg += "\n  range="+state->data->range;
    msg += "\n  multi_pass="+state->data->multi_pass;
    msg += "\n  stats_file="+state->data->stats_file;
    msg += "\n  preset="+state->data->preset;
    msg += "\n  tune="+state->data->tune;
    msg += "\n  open_gop="+bool2string(state->data->open_gop);
    msg += "\n  max_intra_period="+std::to_string(state->data->max_intra_period);
    msg += "\n  min_intra_period="+std::to_string(state->data->min_intra_period);
    msg += "\n  intra_refresh="+bool2string(state->data->intra_refresh);
    msg += "\n  max_bframes="+std::to_string(state->data->max_bframes);
    msg += "\n  scenecut="+std::to_string(state->data->scenecut);
    msg += "\n  scenecut_bias="+std::to_string(state->data->scenecut_bias);
    msg += "\n  lookahead_frames="+std::to_string(state->data->lookahead_frames);
    msg += "\n  info="+bool2string(state->data->info);
    msg += "\n  frame_threads="+std::to_string(state->data->frame_threads);
    msg += "\n  nr_inter="+std::to_string(state->data->nr_inter);
    msg += "\n  nr_intra="+std::to_string(state->data->nr_intra);
    msg += "\n  cbqpoffs="+std::to_string(state->data->cbqpoffs);
    msg += "\n  crqpoffs=" + std::to_string(state->data->crqpoffs);
    msg += "\n  min_cu_size=" + std::to_string(state->data->min_cu_size);
    msg += "\n  max_cu_size=" + std::to_string(state->data->max_cu_size);
    msg += "\n  qg_size=" + std::to_string(state->data->qg_size);
    msg += "\n  rc_grain=" + bool2string(state->data->rc_grain);
    msg += "\n  level_idc=" + state->data->level_idc;
    msg += "\n  psy_rd=" + state->data->psy_rd;
    msg += "\n  wpp=" + bool2string(state->data->wpp);
    msg += "\n  profile=" + state->data->profile;
    
    msg += "\n  color_primaries=" + state->data->color_primaries;
    msg += "\n  transfer_characteristics=" + state->data->transfer_characteristics;

    // master-display
    if (state->data->mastering_display_enabled)
    {
        msg += "\n  mastering_display_sei_x1=" + std::to_string(state->data->mastering_display_sei_x1);
        msg += "\n  mastering_display_sei_y1=" + std::to_string(state->data->mastering_display_sei_y1);
        msg += "\n  mastering_display_sei_x2=" + std::to_string(state->data->mastering_display_sei_x2);
        msg += "\n  mastering_display_sei_y2=" + std::to_string(state->data->mastering_display_sei_y2);
        msg += "\n  mastering_display_sei_x3=" + std::to_string(state->data->mastering_display_sei_x3);
        msg += "\n  mastering_display_sei_y3=" + std::to_string(state->data->mastering_display_sei_y3);
        msg += "\n  mastering_display_sei_wx=" + std::to_string(state->data->mastering_display_sei_wx);
        msg += "\n  mastering_display_sei_wy=" + std::to_string(state->data->mastering_display_sei_wy);
        msg += "\n  mastering_display_sei_max_lum=" + std::to_string(state->data->mastering_display_sei_max_lum);
        msg += "\n  mastering_display_sei_min_lum=" + std::to_string(state->data->mastering_display_sei_min_lum);
    }

    // max-cll
    if (state->data->light_level_enabled)
    {
        msg += "\n  light_level_max_content=" + std::to_string(state->data->light_level_max_content);
        msg += "\n  light_level_max_frame_average=" + std::to_string(state->data->light_level_max_frame_average);
    }

    if (state->data->internal_params.size())
    {
        msg += "\n  additional x265 params:";
        for (auto p : state->data->internal_params)
        {
            if (p.second.empty())
            {
                msg += "\n    "+p.first;
            }
            else
            {
                msg += "\n    "+p.first+"="+p.second;
            }
        }
    }

    msg += "\nx265 native params: ";
    for (auto p : native_params)
    {
        msg +=  " ";
        msg += p.first;
        if (!p.second.empty())
        {
            msg += "=";
            msg += p.second;
        }
    }
}

static
bool
same_param
    (const std::string& p1
    ,const std::string& p2)
{
    if (p1 == p2 || p1 == "no-"+p2 || "no-"+p1 == p2) return true;
    else return false;
}

static
bool
check_change
    (const std::string& p1
    ,const std::string& p2)
{
    const std::list<std::string> forbidden_params = {"annexb", "repeat-headers", "aud", "hrd", "input-csp", "input-res", "fps", "stats", "pass"};
    for (auto p : forbidden_params)
    {
        if (same_param(p, p1) && same_param(p, p2)) return false;
    }
    return true;
}

static
std::string
get_key(const std::string& p)
{
    const std::string prefix("no-");
    if (p.find(prefix) != std::string::npos)
    {
        return p.substr(prefix.size());
    }
    return p;
}

bool
filter_native_params
    (hevc_enc_x265_t* state
    ,std::list<std::pair<std::string,std::string>>& params)
{
    std::string error;
    params.sort([](const std::pair<std::string,std::string>& p1, const std::pair<std::string,std::string>& p2)
                { return get_key(p2.first) > get_key(p1.first); });
    params.erase(std::unique (params.begin(), params.end()), params.end()); // Remove duplicates

    for (size_t i = 0; i < params.size(); i++)
    {
        auto keep = std::next(params.begin(), i);

        //  Multi-pass encoding is handled by DEE framework
        if ("off" == state->data->multi_pass && ("pass" == keep->first || "stats" == keep->first))
        {
            error += "\nParam '" + keep->first + "' cannot be modified.";
        }

        for (size_t j = i + 1; j < params.size(); j++)
        {
            auto cur = std::next(params.begin(), j);
            if (same_param(keep->first, cur->first))
            {
                if (check_change(keep->first, cur->first))
                {
                    keep->first.clear();
                    keep->second.clear();
                    keep = cur;
                }
                else
                {
                    error += "\nParam '" + keep->first + "' cannot be modified.";
                }
            }
        }
    }

    params.erase(std::remove_if(params.begin(), params.end(),
                    [](const std::pair<std::string,std::string>& x)
                    { return x.first.empty(); }), params.end());

    state->data->msg += error;
    return error.empty();
}

std::pair<int,int>
fps_to_num_denom
    (const std::string& fps)
{
    if ("23.976" == fps) return {24000,1001};
    else if ("29.97" == fps) return {30000,1001};
    else if ("59.94" == fps) return {60000, 1001};
    else return {std::stoi(fps), 1};
}
