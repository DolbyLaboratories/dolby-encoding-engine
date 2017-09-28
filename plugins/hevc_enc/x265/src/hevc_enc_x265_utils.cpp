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

#include "hevc_enc_x265_utils.h"

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
}

bool
parse_init_params
    (hevc_enc_x265_t*               state
    ,const hevc_enc_init_params_t*  init_params
    )
{
    state->data->msg.clear();
    for (size_t i = 0; i < init_params->count; i++)
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
        else if ("param" == name)
        {
            std::string param_name, param_value;
            std::size_t found = value.find("=");
            if (found == std::string::npos)
            {
                param_name = value;
            }
            else
            {
                param_name = value.substr(0, found);
                param_value = value.substr(found+1);
            }
            std::pair<std::string, std::string> param;
            param.first = param_name;
            param.second = param_value;
            state->data->internal_params.push_back(param);
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

hevc_enc_nal_type_t
cast_nal_type(const uint32_t type)
{
    hevc_enc_nal_type_t out;
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
    ,std::string&     msg)
{
    msg = "x265 configuration:";
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
    msg += "\n  profile=" + state->data->profile;
    
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
}
