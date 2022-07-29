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

#include "hevc_enc_api.h"
#include "hevc_enc_x265_utils.h"
#include <stdint.h>
#include <map>

static
const
PropertyInfo hevc_enc_x265_info[] = 
{

     { "plugin_path", PROPERTY_TYPE_STRING, "Path to this plugin.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "config_path", PROPERTY_TYPE_STRING, "Path to DEE config file.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT }
    ,{"max_pass_num", PROPERTY_TYPE_INTEGER, "Indicates how many passes encoder can perform (0 = unlimited).", "3", NULL, 0, 1, ACCESS_TYPE_READ}
    ,{"max_output_data", PROPERTY_TYPE_INTEGER, "Limits number of output bytes (0 = unlimited).", "0", NULL, 0, 1, ACCESS_TYPE_WRITE}
    ,{"bit_depth", PROPERTY_TYPE_STRING, NULL, NULL, "8:10", 1, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"width", PROPERTY_TYPE_INTEGER, NULL, NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"height", PROPERTY_TYPE_INTEGER, NULL, NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"color_space", PROPERTY_TYPE_STRING, NULL, "i420", "i400:i420:i422:i444", 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"frame_rate", PROPERTY_TYPE_DECIMAL, NULL, NULL, "23.976:24:25:29.97:30:48:50:59.94:60:119.88:120", 1, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"data_rate", PROPERTY_TYPE_INTEGER, "Average data rate in kbps.", "15000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"max_vbv_data_rate", PROPERTY_TYPE_INTEGER, "Max VBV data rate in kbps.", "15000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"vbv_buffer_size", PROPERTY_TYPE_INTEGER, "VBV buffer size in kb.", "30000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"range", PROPERTY_TYPE_STRING, NULL, "full", "limited:full", 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"multi_pass", PROPERTY_TYPE_STRING, NULL, "off", "off:1st:nth:last", 0, 1, ACCESS_TYPE_WRITE_INIT}
    ,{"stats_file", PROPERTY_TYPE_STRING, NULL, NULL, NULL, 0, 1, ACCESS_TYPE_WRITE_INIT}

    , { "color_primaries", PROPERTY_TYPE_STRING, NULL, "unspecified", "unspecified:bt_709:bt_601_625:bt_601_525:bt_2020", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "transfer_characteristics", PROPERTY_TYPE_STRING, NULL, "unspecified", "unspecified:bt_709:bt_601_625:bt_601_525:smpte_st_2084:std_b67", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "matrix_coefficients", PROPERTY_TYPE_STRING, NULL, "unspecified", "unspecified:bt_709:bt_601_625:bt_601_525:bt_2020", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "chromaloc", PROPERTY_TYPE_INTEGER, NULL, "0", "0:5", 0, 1, ACCESS_TYPE_WRITE_INIT }

    // master-display
    ,{ "mastering_display_sei_x1", PROPERTY_TYPE_INTEGER, "First primary x.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "mastering_display_sei_y1", PROPERTY_TYPE_INTEGER, "First primary y.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "mastering_display_sei_x2", PROPERTY_TYPE_INTEGER, "Second primary x.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "mastering_display_sei_y2", PROPERTY_TYPE_INTEGER, "Second primary y.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "mastering_display_sei_x3", PROPERTY_TYPE_INTEGER, "Third primary x.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "mastering_display_sei_y3", PROPERTY_TYPE_INTEGER, "Third primary y.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "mastering_display_sei_wx", PROPERTY_TYPE_INTEGER, "White point x.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "mastering_display_sei_wy", PROPERTY_TYPE_INTEGER, "White point y.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "mastering_display_sei_max_lum", PROPERTY_TYPE_INTEGER, "Maximum display luminance.", "0", "0:2000000000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "mastering_display_sei_min_lum", PROPERTY_TYPE_INTEGER, "Minimum display luminance.", "0", "0:2000000000", 0, 1, ACCESS_TYPE_WRITE_INIT }

    // max-cll
    ,{ "light_level_max_content", PROPERTY_TYPE_INTEGER, NULL, "0", "0:65535", 0, 1, ACCESS_TYPE_WRITE_INIT }
    ,{ "light_level_max_frame_average", PROPERTY_TYPE_INTEGER, NULL, "0", "0:65535", 0, 1, ACCESS_TYPE_WRITE_INIT }

    ,{ "force_slice_type", PROPERTY_TYPE_BOOLEAN, "Indicates that framework will try to force specific slice type.", "false", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT }

    // Only properties below (ACCESS_TYPE_USER) can be modified 
    ,{"preset", PROPERTY_TYPE_STRING, "Sets parameters to preselected values.", "medium", "placebo:veryslow:slower:slow:medium:fast:faster:veryfast:superfast:ultrafast", 0, 1, ACCESS_TYPE_USER}
    ,{"tune", PROPERTY_TYPE_STRING, "Tune the settings for a particular type of source or situation.", "none", "none:psnr:ssim:grain:fastdecode:zerolatency", 0, 1, ACCESS_TYPE_USER}
    ,{"open_gop", PROPERTY_TYPE_BOOLEAN, "Allows I-slices to be non-IDR.", "false", NULL, 0, 1, ACCESS_TYPE_USER}
    ,{"max_intra_period", PROPERTY_TYPE_INTEGER, "Max intra period in frames.", "25", "1:65535", 0, 1, ACCESS_TYPE_USER}
    ,{"min_intra_period", PROPERTY_TYPE_INTEGER, "Min intra period in frames (0 = auto).", "0", "0:65535", 0, 1, ACCESS_TYPE_USER}
    ,{"intra_refresh", PROPERTY_TYPE_BOOLEAN, "Enables Periodic Intra Refresh(PIR) instead of keyframe insertion.", "false", NULL, 0, 1, ACCESS_TYPE_USER}
    ,{"max_bframes", PROPERTY_TYPE_INTEGER, "Maximum number of consecutive b-frames.", "4", "0:16", 0, 1, ACCESS_TYPE_USER}
    ,{"scenecut", PROPERTY_TYPE_INTEGER, "Defines how aggressively I-frames need to be inserted. The higher the threshold value, the more aggressive the I-frame placement. Value 0 disables adaptive I frame placement", "40", NULL, 0, 1, ACCESS_TYPE_USER}
    ,{"scenecut_bias", PROPERTY_TYPE_INTEGER, "Represents the percentage difference between the inter cost and intra cost of a frame used in scenecut detection. Values between 5 and 15 are recommended.", "5", "0:100", 0, 1, ACCESS_TYPE_USER}
    ,{"lookahead_frames", PROPERTY_TYPE_INTEGER, "Number of frames to look ahead. Must be between the maximum consecutive bframe count and 250.", "20", "0:250", 0, 1, ACCESS_TYPE_USER}
    ,{"concatenation_flag", PROPERTY_TYPE_BOOLEAN, "Set concatenation flag in 1st buffer timing SEI.", "false", "", 0, 1, ACCESS_TYPE_USER }
    ,{"info", PROPERTY_TYPE_BOOLEAN, "Enables informational SEI in the stream headers which describes the encoder version, build info, and encode parameters.", "false", NULL, 0, 1, ACCESS_TYPE_USER }
    ,{"frame_threads", PROPERTY_TYPE_INTEGER, "Number of concurrently encoded frames (0 = autodetect). Value 1 can improve encode quality, but significantly reduces performance.", "0", "0:16", 0, 1, ACCESS_TYPE_USER }
    ,{"nr_inter", PROPERTY_TYPE_INTEGER, "Inter frame noise reduction strength (0 = disabled).", "0", "0:2000", 0, 1, ACCESS_TYPE_USER }
    ,{"nr_intra", PROPERTY_TYPE_INTEGER, "Intra frame noise reduction strength (0 = disabled).", "0", "0:2000", 0, 1, ACCESS_TYPE_USER }
    ,{"cbqpoffs", PROPERTY_TYPE_INTEGER, "Offset of Cb chroma QP from the luma QP selected by rate control.", "0", "-12:12", 0, 1, ACCESS_TYPE_USER }
    ,{"crqpoffs", PROPERTY_TYPE_INTEGER, "Offset of Cr chroma QP from the luma QP selected by rate control.", "0", "-12:12", 0, 1, ACCESS_TYPE_USER }

    ,{"min_cu_size", PROPERTY_TYPE_STRING, "Minimum CU size (width and height).", "8", "8:16:32", 0, 1, ACCESS_TYPE_USER }
    ,{"max_cu_size", PROPERTY_TYPE_STRING, "Maximum CU size (width and height).", "64", "16:32:64", 0, 1, ACCESS_TYPE_USER }
    ,{"qg_size", PROPERTY_TYPE_STRING, "Enable adaptive quantization for sub-CTUs. ", "64", "16:32:64", 0, 1, ACCESS_TYPE_USER }
    ,{"rc_grain", PROPERTY_TYPE_BOOLEAN, "Enables a specialized ratecontrol algorithm for film grain content.", "false", NULL, 0, 1, ACCESS_TYPE_USER }
    ,{"level_idc", PROPERTY_TYPE_STRING, "Minimum decoder requirement level.", "0", "0:1.0:10:2.0:20:2.1:21:3.0:30:3.1:31:4.0:40:4.1:41:5.0:50:5.1:51:5.2:52:6.0:60:6.1:61:6.2:62:8.5:85", 0, 1, ACCESS_TYPE_USER }
    ,{"psy_rd", PROPERTY_TYPE_DECIMAL, "Influence rate distortion optimized mode decision to preserve the energy of the source image in the encoded image at the expense of compression efficiency.", "2.0", "0:5", 0, 1, ACCESS_TYPE_USER }
    ,{"wpp", PROPERTY_TYPE_BOOLEAN, "Enable Wavefront Parallel Processing.", "false", NULL, 0, 1, ACCESS_TYPE_USER }

    ,{"profile", PROPERTY_TYPE_STRING, "Enforce the requirements of the specified HEVC profile", "auto", "auto:main:main10:main-intra:main10-intra:main444-8:main444-intra:main422-10:main422-10-intra:main444-10:main444-10-intra:main12:main12-intra:main422-12:main422-12-intra:main444-12:main444-12-intra", 0, 1, ACCESS_TYPE_USER }
    ,{"param", PROPERTY_TYPE_STRING, "Sets any x265 parameter using syntax \"name=value\" or just \"name\" for boolean flags. Use ':' separator to enter multiple values under one tag. If value contains colon, use semicolon instead.", NULL, NULL, 0, 100, ACCESS_TYPE_USER}
};

static
size_t
x265_get_info
    (const PropertyInfo** info
    )
{
    *info = hevc_enc_x265_info;
    return sizeof(hevc_enc_x265_info)/sizeof(PropertyInfo);
}

static
size_t
x265_get_size()
{
    return sizeof(hevc_enc_x265_t);
}

static
Status
x265_init
    (HevcEncHandle handle
    ,const HevcEncInitParams* init_params
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
        std::string errmsg = "An exception occurred when parsing init params. " + state->data->msg;
        state->data->msg = errmsg;
        return STATUS_ERROR;
    }
    

    if (0 == state->data->max_output_data)
    {
        state->data->max_output_data = INT32_MAX;
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
    if (!api)
    {
        state->data->msg = "x265_api_get() failed for " + std::to_string(state->data->bit_depth) + " bit.";
        return STATUS_ERROR;
    }

    state->param = api->param_alloc();

    if (!state->param)
    {
        state->data->msg = "param_alloc() failed.";
        return STATUS_ERROR;
    }

    api->param_default(state->param);
    state->param->forceFlush = 0;

    bool ok = set_preset(state, state->data->preset, state->data->tune);
    
    std::list<std::pair<std::string,std::string>> native_params;
    
    if(state->data->force_slice_type)
    {
        state->data->max_intra_period = 65535;
        state->data->min_intra_period = 0;
    }

    auto num_denom = fps_to_num_denom(state->data->frame_rate);
    std::string fps_arg = std::to_string(num_denom.first) + "/" + std::to_string(num_denom.second);

    native_params.push_back({"annexb", ""});
    native_params.push_back({"repeat-headers", ""});
    native_params.push_back({"aud", ""});
    native_params.push_back({"hrd", ""});
    native_params.push_back({"hash", "1"});
    native_params.push_back({"log-level", "0"});
    native_params.push_back({"sar", "1"});
    native_params.push_back({"input-csp", state->data->color_space});
    native_params.push_back({"input-res", std::to_string(state->data->width)+"x"+std::to_string(state->data->height)}); 
    native_params.push_back({"fps", fps_arg});
    native_params.push_back({state->data->open_gop ? "open-gop" : "no-open-gop", ""});
    native_params.push_back({state->data->info ? "info" : "no-info", ""});
    native_params.push_back({"keyint", std::to_string(state->data->max_intra_period)});
    native_params.push_back({"min-keyint", std::to_string(state->data->min_intra_period)});
    native_params.push_back({"bframes", std::to_string(state->data->max_bframes)});
    native_params.push_back({"rc-lookahead", std::to_string(state->data->lookahead_frames)});
    if (state->data->intra_refresh) native_params.push_back({"intra-refresh", ""});

    native_params.push_back({"bitrate", std::to_string(state->data->data_rate)}); 
    native_params.push_back({"vbv-maxrate", std::to_string(state->data->max_vbv_data_rate)}); 
    native_params.push_back({"vbv-bufsize", std::to_string(state->data->vbv_buffer_size)}); 

    native_params.push_back({"range", state->data->range});

    native_params.push_back({"frame-threads", std::to_string(state->data->frame_threads)});
    native_params.push_back({"nr-inter", std::to_string(state->data->nr_inter)});
    native_params.push_back({"nr-intra", std::to_string(state->data->nr_intra)});
    native_params.push_back({"cbqpoffs", std::to_string(state->data->cbqpoffs)});
    native_params.push_back({"crqpoffs", std::to_string(state->data->crqpoffs)});

    native_params.push_back({"scenecut", std::to_string(state->data->scenecut)});
    native_params.push_back({"scenecut-bias", std::to_string(state->data->scenecut_bias)});

    native_params.push_back({"min-cu-size", std::to_string(state->data->min_cu_size)});
    native_params.push_back({"ctu", std::to_string(state->data->max_cu_size)});
    native_params.push_back({"qg-size", std::to_string(state->data->qg_size)});
    native_params.push_back({state->data->rc_grain ? "rc-grain" : "no-rc-grain", ""});
    native_params.push_back({"level-idc", state->data->level_idc});
    native_params.push_back({state->data->wpp ? "wpp" : "no-wpp", ""});
    native_params.push_back({"psy-rd", state->data->psy_rd});
    native_params.push_back({"colorprim", std::to_string(get_color_prim_number(state->data->color_primaries))});
    native_params.push_back({"transfer", std::to_string(get_transfer_characteristics_number(state->data->transfer_characteristics))});
    native_params.push_back({"colormatrix", std::to_string(get_matrix_coefficients_number(state->data->matrix_coefficients))});
    native_params.push_back({"chromaloc", state->data->chromaSampleLocation});
    
    if (state->data->concatenation_flag) native_params.push_back({"hrd-concat", ""});
    

     if (state->data->mastering_display_enabled == true)
    {
        // --master-display G(gx,gy)B(bx,by)R(rx,ry)WP(wpx,wpy)L(max_peak_lum,min_peak_lum)
        std::string master_display =
            "G(" + std::to_string(state->data->mastering_display_sei_x1) + "," + std::to_string(state->data->mastering_display_sei_y1) + ")"
            + "B(" + std::to_string(state->data->mastering_display_sei_x2) + "," + std::to_string(state->data->mastering_display_sei_y2) + ")"
            + "R(" + std::to_string(state->data->mastering_display_sei_x3) + "," + std::to_string(state->data->mastering_display_sei_y3) + ")"
            + "WP(" + std::to_string(state->data->mastering_display_sei_wx) + "," + std::to_string(state->data->mastering_display_sei_wy) + ")"
            + "L(" + std::to_string(state->data->mastering_display_sei_max_lum) + "," + std::to_string(state->data->mastering_display_sei_min_lum) + ")";
        native_params.push_back({"master-display", master_display});
    }

    if (state->data->light_level_enabled == true)
    {
        // --max-cll max_cll,max_fall
        std::string max_cll = std::to_string(state->data->light_level_max_content) + "," + std::to_string(state->data->light_level_max_frame_average);
        native_params.push_back({"max-cll", max_cll});
    }

    if (state->data->uhd_bd == true)
    {
        native_params.push_back({"level-idc", "5.1"});
        native_params.push_back({"uhd-bd", ""});
        native_params.push_back({"b-adapt", "0"});
    }

    for (auto ip : state->data->internal_params)
    {
        native_params.push_back({ip.first, ip.second});
    }
    
    if (state->data->multi_pass != "off")
    {
        if (state->data->multi_pass == "1st") native_params.push_back({"pass", "1"});
        else if (state->data->multi_pass == "nth") native_params.push_back({"pass", "3"});
        else if (state->data->multi_pass == "last") native_params.push_back({"pass", "2"});
    
        if (!state->data->stats_file.empty()) native_params.push_back({"stats", state->data->stats_file});
    }

    if (!filter_native_params(state, native_params))
    {
        return STATUS_ERROR; 
    }

    for (auto p : native_params)
    {
        if (!p.second.empty())
        {
            ok &= set_param(state, p.first, p.second);
        }
        else
        {
            ok &= set_param(state, p.first, "");
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

    get_config_msg(state, native_params, state->data->msg);
    return STATUS_OK;
}

static
Status
x265_close
    (HevcEncHandle handle
    )
{
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;
    state->data->msg.clear();

    if (state->lib_initialized)
    {
        state->api->encoder_close(state->encoder);
        state->api->cleanup();
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
Status
x265_process
    (HevcEncHandle          handle
    ,const HevcEncPicture*  picture
    ,const size_t           picture_num
    ,HevcEncOutput*         output
    )
{
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;
    
    x265_nal *p_nal;
    uint32_t nal_count = 0;
    state->data->msg.clear();

    if (state->data->pending_header && !state->param->bRepeatHeaders)
    {
        state->data->pending_header = false;
        if (state->api->encoder_headers(state->encoder, &p_nal, &nal_count) < 0)
        {
            state->data->msg = "Failure generating stream headers.";
            return STATUS_ERROR;
        }
        if (nal_count)
        {
            for (uint32_t j = 0; j < nal_count; j++)
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
        input_picture.bitDepth = picture[i].bitDepth;
        
        if (input_picture.bitDepth != state->data->bit_depth)
        {
            state->data->msg = "Bit depth mismatch.";
            return STATUS_ERROR;
        }

        input_picture.colorSpace = picture[i].colorSpace;
        input_picture.sliceType = frametype_to_slicetype(picture[i].frameType);
        
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


        /*  When encoding Dolby Vision profile 7 (BD), placement of B/P frames must be deterministic.
            This can be achieved by forcing B frame every time.
            IDRs and P frames are still emitted, but only at known positions.
            It should change the interpretation of option "bframes", from "max", to "exact" number. */
        if (true == state->data->uhd_bd)
            input_picture.sliceType = X265_TYPE_B;

        int num_encoded = state->api->encoder_encode(state->encoder, &p_nal, &nal_count, &input_picture, NULL);
        if (num_encoded < 0)
        {
            state->data->msg = "encoder_encode() failed.";
            return STATUS_ERROR;
        }
        for (uint32_t j = 0; j < nal_count; j++)
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
        HevcEncNal nal;
        nal.type = state->data->output_buffer[nal_index].type;
        nal.payload = (void*)state->data->output_buffer[nal_index].payload.data();
        nal.size = state->data->output_buffer[nal_index].payload.size();
        state->data->output.push_back(nal);
        size_acc += nal.size;
        nal_index += 1;
        state->data->last_used_nal++;
    }

    output->nal = state->data->output.data();
    output->nalNum = state->data->output.size();
    return STATUS_OK;
}

static
Status
x265_flush
    (HevcEncHandle    	handle
    ,HevcEncOutput*     output
    ,int*               is_empty
    )
{
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;

    if (state->data->last_used_nal != state->data->output_buffer.begin())
    {
        state->data->output_buffer.erase(state->data->output_buffer.begin(), state->data->last_used_nal);
    }

    x265_nal *p_nal;
    uint32_t nal_count = 0;
    int num_encoded = state->api->encoder_encode(state->encoder, &p_nal, &nal_count, NULL, NULL);
    if (num_encoded < 0)
    {
        state->data->msg = "encoder_encode() failed.";
        return STATUS_ERROR;
    }
    else if (0 == num_encoded)
    {
        *is_empty = 1;
        output->nalNum = 0;
        output->nal = NULL;
        return STATUS_OK;
    }

    for (uint32_t j = 0; j < nal_count; j++)
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
        HevcEncNal nal;
        nal.type = state->data->output_buffer[nal_index].type;
        nal.payload = (void*)state->data->output_buffer[nal_index].payload.data();
        nal.size = state->data->output_buffer[nal_index].payload.size();
        state->data->output.push_back(nal);
        size_acc += nal.size;
        nal_index += 1;
        state->data->last_used_nal++;
    }

    output->nal = state->data->output.data();
    output->nalNum = state->data->output.size();
    *is_empty = 0;

    return STATUS_OK;
}

static
Status
x265_set_property
    (HevcEncHandle   handle
    ,const Property* property
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
Status
x265_get_property
    (HevcEncHandle handle
    ,Property* property
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
    (HevcEncHandle handle
    )
{
    hevc_enc_x265_t* state = (hevc_enc_x265_t*)handle;
    return state->data->msg.empty() ? NULL : state->data->msg.c_str();
}

static 
HevcEncApi x265_plugin_api = 
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
HevcEncApi* hevcEncGetApi()
{
    return &x265_plugin_api;
}

DLB_EXPORT
int hevcEncGetApiVersion()
{
    return HEVC_ENC_API_VERSION;
}
