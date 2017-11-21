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

#include "hevc_enc_api.h"
#include "hevc_enc_x265_utils.h"
#include "x265.h"
#include <stdint.h>

static
const
property_info_t hevc_enc_x265_info[] = 
{
    
     {"max_pass_num", PROPERTY_TYPE_INTEGER, "Indicates how many passes encoder can perform (0 = unlimited).", "3", NULL, 0, 1, ACCESS_TYPE_READ}
    ,{"max_output_data", PROPERTY_TYPE_INTEGER, "Limits number of output bytes (0 = unlimited).", "0", NULL, 0, 1, ACCESS_TYPE_WRITE}
    ,{"bit_depth", PROPERTY_TYPE_STRING, NULL, NULL, "8:10", 1, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"width", PROPERTY_TYPE_INTEGER, NULL, NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"height", PROPERTY_TYPE_INTEGER, NULL, NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"color_space", PROPERTY_TYPE_STRING, NULL, "i420", "i400:i420:i422:i444", 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"frame_rate", PROPERTY_TYPE_DECIMAL, NULL, NULL, "23.976:24:25:29.97:30:48:59.94:60", 1, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"data_rate", PROPERTY_TYPE_INTEGER, "Average data rate in kbps.", "15000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"max_vbv_data_rate", PROPERTY_TYPE_INTEGER, "Max VBV data rate in kbps.", "15000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"vbv_buffer_size", PROPERTY_TYPE_INTEGER, "VBV buffer size in kb.", "30000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"range", PROPERTY_TYPE_STRING, NULL, "full", "limited:full", 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"multi_pass", PROPERTY_TYPE_STRING, NULL, "off", "off:1st:nth:last", 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"stats_file", PROPERTY_TYPE_STRING, NULL, NULL, NULL, 0, 1, ACCESS_TYPE_WRITE_INIT}
    // Only properties below (ACCESS_TYPE_USER) can be modified 
    ,{"preset", PROPERTY_TYPE_STRING, "Sets parameters to preselected values.", "medium", "ultrafast:superfast:veryfast:faster:fast:medium:slow:slower:veryslow:placebo", 0, 1, ACCESS_TYPE_USER}
    ,{"tune", PROPERTY_TYPE_STRING, "Tune the settings for a particular type of source or situation.", "none", "none:psnr:ssim:grain:fastdecode:zerolatency", 0, 1, ACCESS_TYPE_USER}
    ,{"open_gop", PROPERTY_TYPE_BOOLEAN, "Allows I-slices to be non-IDR.", "false", NULL, 0, 1, ACCESS_TYPE_USER}
    ,{"max_intra_period", PROPERTY_TYPE_INTEGER, "Max intra period in frames.", "25", "1:65535", 0, 1, ACCESS_TYPE_USER}
    ,{"min_intra_period", PROPERTY_TYPE_INTEGER, "Min intra period in frames (0 = auto).", "0", "0:65535", 0, 1, ACCESS_TYPE_USER}
    ,{"intra_refresh", PROPERTY_TYPE_BOOLEAN, "Enables Periodic Intra Refresh(PIR) instead of keyframe insertion.", "false", NULL, 0, 1, ACCESS_TYPE_USER}
    ,{"max_bframes", PROPERTY_TYPE_INTEGER, "Maximum number of consecutive b-frames.", "4", "0:16", 0, 1, ACCESS_TYPE_USER}
    ,{"scenecut", PROPERTY_TYPE_INTEGER, "Defines how aggressively I-frames need to be inserted. The higher the threshold value, the more aggressive the I-frame placement. Value 0 disables adaptive I frame placement", "40", NULL, 0, 1, ACCESS_TYPE_USER}
    ,{"scenecut_bias", PROPERTY_TYPE_INTEGER, "Represents the percentage difference between the inter cost and intra cost of a frame used in scenecut detection. Values between 5 and 15 are recommended.", "5", "0:100", 0, 1, ACCESS_TYPE_USER}
    ,{"lookahead_frames", PROPERTY_TYPE_INTEGER, "Number of frames to look ahead. Must be between the maximum consecutive bframe count and 250.", "20", "0:250", 0, 1, ACCESS_TYPE_USER}
    ,{"info", PROPERTY_TYPE_BOOLEAN, "Enables informational SEI in the stream headers which describes the encoder version, build info, and encode parameters.", "false", NULL, 0, 1, ACCESS_TYPE_USER }
    ,{"frame_threads", PROPERTY_TYPE_INTEGER, "Number of concurrently encoded frames (0 = autodetect).", "0", "0:16", 0, 1, ACCESS_TYPE_USER }
    ,{"nr_inter", PROPERTY_TYPE_INTEGER, "Inter frame noise reduction strength (0 = disabled).", "0", "0:2000", 0, 1, ACCESS_TYPE_USER }
    ,{"nr_intra", PROPERTY_TYPE_INTEGER, "Intra frame noise reduction strength (0 = disabled).", "0", "0:2000", 0, 1, ACCESS_TYPE_USER }
    ,{"cbqpoffs", PROPERTY_TYPE_INTEGER, "Offset of Cb chroma QP from the luma QP selected by rate control.", "0", "-12:12", 0, 1, ACCESS_TYPE_USER }
    ,{"crqpoffs", PROPERTY_TYPE_INTEGER, "Offset of Cr chroma QP from the luma QP selected by rate control.", "0", "-12:12", 0, 1, ACCESS_TYPE_USER }

    ,{"min_cu_size", PROPERTY_TYPE_STRING, "Minimum CU size (width and height).", "8", "8:16:32", 0, 1, ACCESS_TYPE_USER }
    ,{"max_cu_size", PROPERTY_TYPE_STRING, "Maximum CU size (width and height).", "64", "16:32:64", 0, 1, ACCESS_TYPE_USER }
    ,{"qg_size", PROPERTY_TYPE_STRING, "Enable adaptive quantization for sub-CTUs. ", "64", "16:32:64", 0, 1, ACCESS_TYPE_USER }
    ,{"rc_grain", PROPERTY_TYPE_BOOLEAN, "Enables a specialised ratecontrol algorithm for film grain content.", "false", NULL, 0, 1, ACCESS_TYPE_USER }
    ,{"level_idc", PROPERTY_TYPE_STRING, "Minimum decoder requirement level.", "0", "0:1.0:10:2.0:20:2.1:21:3.0:30:3.1:31:4.0:40:4.1:41:5.0:50:5.1:51:5.2:52:6.0:60:6.1:61:6.2:62:8.5:85", 0, 1, ACCESS_TYPE_USER }
    ,{"psy_rd", PROPERTY_TYPE_DECIMAL, "Influence rate distortion optimizated mode decision to preserve the energy of the source image in the encoded image at the expense of compression efficiency.", "2.0", "0:5", 0, 1, ACCESS_TYPE_USER }

    ,{"profile", PROPERTY_TYPE_STRING, "Enforce the requirements of the specified HEVC profile", "auto", "auto:main:main10:main-intra:main10-intra:main444-8:main444-intra:main422-10:main422-10-intra:main444-10:main444-10-intra:main12:main12-intra:main422-12:main422-12-intra:main444-12:main444-12-intra", 0, 1, ACCESS_TYPE_USER }
    ,{"param", PROPERTY_TYPE_STRING, "Sets any x265 parameter using syntax \"name=value\" or just \"name\" for boolean flags.", NULL, NULL, 0, 100, ACCESS_TYPE_USER}
};

static
size_t
x265_get_info
    (const property_info_t** info
    )
{
    *info = hevc_enc_x265_info;
    return sizeof(hevc_enc_x265_info)/sizeof(property_info_t);
}

static
size_t
x265_get_size()
{
    return sizeof(hevc_enc_x265_t);
}

static
status_t
x265_init
    (hevc_enc_handle_t handle
    ,const hevc_enc_init_params_t* init_params
    )
{
    const x265_api* api;
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;
    state->data = new hevc_enc_x265_data_t;
    init_defaults(state);
    state->lib_initialized = false;
    state->data->pending_header = true;
    state->data->msg.clear();
    state->data->last_used_nal = state->data->output_buffer.begin();

    try 
    {
        if (!parse_init_params(state, init_params))
        {
            std::string errmsg = "Parsing init params failed:" + state->data->msg;
            state->data->msg = errmsg;
            return STATUS_ERROR;
        }
    }
    catch (...) 
    {
        std::string errmsg = "An exception occured when parsing init params. " + state->data->msg;
        state->data->msg = errmsg;
        return STATUS_ERROR;
    }
    

    if (0 == state->data->max_output_data)
    {
        state->data->max_output_data = SIZE_MAX;
    }

    // CRF mode does not support 'hrd' option, which is required
    if (0 == state->data->data_rate)
    {
        state->data->msg = "CRF encoding is not supported.";
        return STATUS_ERROR;
    }

    if (state->data->qg_size < state->data->min_cu_size || state->data->qg_size > state->data->max_cu_size)
    {
        state->data->msg = "qg_size must be a value between min_cu_size and max_cu_size.";
        return STATUS_ERROR;
    }

    api = state->api = x265_api_get(state->data->bit_depth);
    state->param = api->param_alloc();

    if (!state->param)
    {
        state->data->msg = "param_alloc() failed.";
        return STATUS_ERROR;
    }

    api->param_default(state->param);

    bool ok = set_preset(state, state->data->preset, state->data->tune);
    
    ok &= set_param(state, "annexb", "");
    ok &= set_param(state, "input-res", std::to_string(state->data->width)+"x"+std::to_string(state->data->height));
    ok &= set_param(state, "input-csp", state->data->color_space);
    ok &= set_param(state, "fps", state->data->frame_rate);
    
    if (state->data->open_gop) ok &= set_param(state, "open-gop", "");
    else                       ok &= set_param(state, "no-open-gop", "");

    if (state->data->info) ok &= set_param(state, "info", "");
    else                       ok &= set_param(state, "no-info", "");

    ok &= set_param(state, "keyint", std::to_string(state->data->max_intra_period));
    
    if (state->data->intra_refresh) ok &= set_param(state, "intra-refresh", "");

    ok &= set_param(state, "bframes", std::to_string(state->data->max_bframes));
    ok &= set_param(state, "min-keyint", std::to_string(state->data->min_intra_period));
    ok &= set_param(state, "rc-lookahead", std::to_string(state->data->lookahead_frames));
    ok &= set_param(state, "bitrate", std::to_string(state->data->data_rate));
    ok &= set_param(state, "vbv-maxrate", std::to_string(state->data->max_vbv_data_rate));
    ok &= set_param(state, "vbv-bufsize", std::to_string(state->data->vbv_buffer_size));

    ok &= set_param(state, "frame-threads", std::to_string(state->data->frame_threads));
    ok &= set_param(state, "nr-inter", std::to_string(state->data->nr_inter));
    ok &= set_param(state, "nr-intra", std::to_string(state->data->nr_intra));
    ok &= set_param(state, "cbqpoffs", std::to_string(state->data->cbqpoffs));
    ok &= set_param(state, "crqpoffs", std::to_string(state->data->crqpoffs));

    ok &= set_param(state, "scenecut", std::to_string(state->data->scenecut));
    ok &= set_param(state, "scenecut_bias", std::to_string(state->data->scenecut_bias));

    ok &= set_param(state, "repeat-headers", "");
    ok &= set_param(state, "aud", "");
    ok &= set_param(state, "hrd", "");
    ok &= set_param(state, "hash", "1");
    ok &= set_param(state, "range", state->data->range);

    ok &= set_param(state, "min-cu-size", std::to_string(state->data->min_cu_size));
    ok &= set_param(state, "ctu", std::to_string(state->data->max_cu_size));
    ok &= set_param(state, "qg-size", std::to_string(state->data->qg_size));

    if (state->data->rc_grain) ok &= set_param(state, "rc-grain", "");
    else                       ok &= set_param(state, "no-rc-grain", "");

    ok &= set_param(state, "level-idc", state->data->level_idc);
    ok &= set_param(state, "psy-rd", state->data->psy_rd);
 
    if (state->data->multi_pass != "off")
    {
        if (state->data->multi_pass == "1st") ok &= set_param(state, "pass", "1");
        else if (state->data->multi_pass == "nth") ok &= set_param(state, "pass", "3");
        else if (state->data->multi_pass == "last") ok &= set_param(state, "pass", "2");
    
        if (!state->data->stats_file.empty()) ok &= set_param(state, "stats", state->data->stats_file);
    }

    for (auto ip : state->data->internal_params)
    {
        if (!ip.second.empty())
        {
            ok &= set_param(state, ip.first, ip.second);
        }
        else
        {
            ok &= set_param(state, ip.first, "");
        }
    }

    if (state->data->profile != "auto")
    {
        if (x265_param_apply_profile(state->param, state->data->profile.c_str()) != 0)
        {
            std::string errmsg = "Setting profile " + state->data->profile + "failed.";
            state->data->msg = errmsg;
            return STATUS_ERROR;
        }
    }

    if (!ok)
    {
        std::string errmsg = "Setting params failed:" + state->data->msg;
        state->data->msg = errmsg;
        return STATUS_ERROR;
    }

    state->encoder = api->encoder_open(state->param);
    if (!state->encoder)
    {
        state->data->msg = "encoder_open() failed.";
        return STATUS_ERROR;
    }
    state->lib_initialized = true;

    get_config_msg(state, state->data->msg);
    return STATUS_OK;
}

static
status_t
x265_close
    (hevc_enc_handle_t handle
    )
{
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;

    if (state->lib_initialized)
    {
        state->api->encoder_close(state->encoder);
        state->lib_initialized = false;
    }

    // Stats file is set and removed by caller,
    // but cutree file has to be handled by plugin.
    if (!state->data->stats_file.empty() && "last" == state->data->multi_pass)
    {
        std::string cutree_file = state->data->stats_file + ".cutree";
        std::remove(cutree_file.c_str());
    }

    if (state->data) delete state->data;
    if (state->param) state->api->param_free(state->param);

  
    return STATUS_OK;
}

static
status_t
x265_process
    (hevc_enc_handle_t          handle
    ,const hevc_enc_picture_t*  picture
    ,const size_t               picture_num
    ,hevc_enc_output_t*         output
    )
{
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;
    
    x265_nal *p_nal;
    uint32_t nal;

    if (state->data->pending_header && !state->param->bRepeatHeaders)
    {
        state->data->pending_header = false;
        if (state->api->encoder_headers(state->encoder, &p_nal, &nal) < 0)
        {
            state->data->msg = "Failure generating stream headers.";
            return STATUS_ERROR;
        }
        if (nal)
        {
            for (int j = 0; j < (int)nal; j++)
            {
                nalu_t nalu;
                nalu.type = cast_nal_type(p_nal[j].type);
                nalu.payload.resize(p_nal[j].sizeBytes);
                memcpy(nalu.payload.data(), p_nal[j].payload, p_nal[j].sizeBytes);
                state->data->output_buffer.push_back(nalu);
            }
        }
    }
    else if (state->data->last_used_nal != state->data->output_buffer.begin())
    {
        state->data->output_buffer.erase(state->data->output_buffer.begin(), state->data->last_used_nal);
    }
    
    x265_picture input_picture;
    state->api->picture_init(state->param, &input_picture);
    for (size_t i = 0; i < picture_num; i++)
    {
        input_picture.bitDepth = picture[i].bit_depth;
        
        if (input_picture.bitDepth != state->data->bit_depth)
        {
            state->data->msg = "Bit depth mismatch.";
            return STATUS_ERROR;
        }

        input_picture.colorSpace = picture[i].color_space;
        input_picture.sliceType = picture[i].frame_type;
        
        int width = (int)picture[i].width;
        int height = (int)picture[i].height;
        uint32_t framesize = 0;
        uint32_t pixelbytes = input_picture.bitDepth > 8 ? 2 : 1;
        for (int j = 0; j < x265_cli_csps[input_picture.colorSpace].planes; j++)
        {
            uint32_t w = width >> x265_cli_csps[input_picture.colorSpace].width[j];
            uint32_t h = height >> x265_cli_csps[input_picture.colorSpace].height[j];
            framesize += w * h * pixelbytes;
        }
        
        input_picture.height = height;
        input_picture.framesize = framesize;
        input_picture.stride[0] = picture[i].stride[0];
        input_picture.stride[1] = picture[i].stride[1];
        input_picture.stride[2] = picture[i].stride[2];
        input_picture.planes[0] = picture[i].plane[0];
        input_picture.planes[1] = picture[i].plane[1];
        input_picture.planes[2] = picture[i].plane[2];

        int num_encoded = state->api->encoder_encode(state->encoder, &p_nal, &nal, &input_picture, NULL);
        if (num_encoded < 0)
        {
            state->data->msg = "encoder_encode() failed.";
            return STATUS_ERROR;
        }
        for (uint32_t j = 0; j < nal; j++)
        {
            nalu_t nalu;
            nalu.type = cast_nal_type(p_nal[j].type);
            nalu.payload.resize(p_nal[j].sizeBytes);
            memcpy(nalu.payload.data(), p_nal[j].payload, p_nal[j].sizeBytes);
            state->data->output_buffer.push_back(nalu);
        }
        
    }

    size_t size_acc = 0;
    int nal_index = 0;
    state->data->last_used_nal = state->data->output_buffer.begin();

    state->data->output.clear();
    while (size_acc < state->data->max_output_data && nal_index < (int)state->data->output_buffer.size())
    {
        hevc_enc_nal_t nal;
        nal.type = state->data->output_buffer[nal_index].type;
        nal.payload = (void*)state->data->output_buffer[nal_index].payload.data();
        nal.size = state->data->output_buffer[nal_index].payload.size();
        state->data->output.push_back(nal);
        size_acc += nal.size;
        nal_index += 1;
        state->data->last_used_nal++;
    }

    output->nal = state->data->output.data();
    output->nal_num = state->data->output.size();
    return STATUS_OK;
}

static
status_t
x265_flush
    (hevc_enc_handle_t      handle
    ,hevc_enc_output_t*     output
    ,int*                   is_empty
    )
{
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;

    if (state->data->last_used_nal != state->data->output_buffer.begin())
    {
        state->data->output_buffer.erase(state->data->output_buffer.begin(), state->data->last_used_nal);
    }

    x265_nal *p_nal;
    uint32_t nal;
    int num_encoded = state->api->encoder_encode(state->encoder, &p_nal, &nal, NULL, NULL);
    
    if (num_encoded < 0)
    {
        state->data->msg = "encoder_encode() failed.";
        return STATUS_ERROR;
    }
    else if (0 == num_encoded)
    {
        *is_empty = 1;
        output->nal_num = 0;
        output->nal = NULL;
        return STATUS_OK;
    }

    for (uint32_t j = 0; j < nal; j++)
    {
        nalu_t nalu;
        nalu.type = cast_nal_type(p_nal[j].type);
        nalu.payload.resize(p_nal[j].sizeBytes);
        memcpy(nalu.payload.data(), p_nal[j].payload, p_nal[j].sizeBytes);
        state->data->output_buffer.push_back(nalu);
    }
  
    size_t size_acc = 0;
    int nal_index = 0;
    state->data->last_used_nal = state->data->output_buffer.begin();

    state->data->output.clear();
    while (size_acc < state->data->max_output_data && nal_index < (int)state->data->output_buffer.size())
    {
        hevc_enc_nal_t nal;
        nal.type = state->data->output_buffer[nal_index].type;
        nal.payload = (void*)state->data->output_buffer[nal_index].payload.data();
        nal.size = state->data->output_buffer[nal_index].payload.size();
        state->data->output.push_back(nal);
        size_acc += nal.size;
        nal_index += 1;
        state->data->last_used_nal++;
    }

    output->nal = state->data->output.data();
    output->nal_num = state->data->output.size();
    *is_empty = 0;

    return STATUS_OK;
}

static
status_t
x265_set_property
    (hevc_enc_handle_t handle
    ,const property_t* property
    )
{
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;
    if (NULL == state) return STATUS_ERROR;
    
    if (property->name == std::string("max_output_data"))
    {
        state->data->max_output_data = atoi(property->value);
        return STATUS_OK;
    }

    return STATUS_ERROR;
}

static
status_t
x265_get_property
    (hevc_enc_handle_t handle
    ,property_t* property
    )
{
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;
    if (NULL == state) return STATUS_ERROR;
    
    if (NULL != property->name)
    {
        std::string name(property->name);
        if ("max_pass_num" == name)
        {
            strcpy(property->value, "3");
            return STATUS_OK;
        }
    }
    return STATUS_ERROR;
}

static
const char*
x265_get_message
    (hevc_enc_handle_t handle
    )
{
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;
    return state->data->msg.empty() ? NULL : state->data->msg.c_str();
}

static 
hevc_enc_api_t x265_plugin_api = 
{    
    "x265"
    ,x265_get_info
    ,x265_get_size 
    ,x265_init
    ,x265_close
    ,x265_process
    ,x265_flush
    ,x265_set_property
    ,x265_get_property
    ,x265_get_message
};

DLB_EXPORT
hevc_enc_api_t* hevc_enc_get_api()
{
    return &x265_plugin_api;
}
