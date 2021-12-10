/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2019, Dolby Laboratories
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
#include "hevc_enc_beamr_impl.h"
#include "hevc_enc_beamr_utils.h"

#include <fstream>

static const std::string pluginName{"beamr"};

static const PropertyInfo propertyInfo[] = {
    {"plugin_path", PROPERTY_TYPE_STRING, "Path to this plugin.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT},
    {"config_path", PROPERTY_TYPE_STRING, "Path to DEE config file.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT},
    {"max_pass_num", PROPERTY_TYPE_INTEGER, "Indicates how many passes encoder can perform (0 = unlimited).", "3", NULL,
     0, 1, ACCESS_TYPE_READ},
    {"absolute_pass_num", PROPERTY_TYPE_INTEGER, NULL, NULL, NULL, 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"max_output_data", PROPERTY_TYPE_INTEGER, "Limits number of output bytes (0 = unlimited).", "0", NULL, 0, 1,
     ACCESS_TYPE_WRITE},
    {"bit_depth", PROPERTY_TYPE_STRING, NULL, NULL, "8:10", 1, 1, ACCESS_TYPE_WRITE_INIT},
    {"width", PROPERTY_TYPE_INTEGER, NULL, NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT},
    {"height", PROPERTY_TYPE_INTEGER, NULL, NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT},
    {"color_space", PROPERTY_TYPE_STRING, NULL, "i420", "i400:i420:i422:i444", 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"frame_rate", PROPERTY_TYPE_DECIMAL, NULL, NULL, "23.976:24:25:29.97:30:48:50:59.94:60", 1, 1,
     ACCESS_TYPE_WRITE_INIT},
    {"data_rate", PROPERTY_TYPE_INTEGER, "Average data rate in kbps.", "15000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"max_vbv_data_rate", PROPERTY_TYPE_INTEGER, "Max VBV ctrl rate in kbps.", "15000", NULL, 0, 1,
     ACCESS_TYPE_WRITE_INIT},
    {"vbv_buffer_size", PROPERTY_TYPE_INTEGER, "VBV buffer size in kb.", "30000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"range", PROPERTY_TYPE_STRING, NULL, "full", "limited:full", 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"multi_pass", PROPERTY_TYPE_STRING, NULL, "off", "off:1st:nth:last", 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"stats_file", PROPERTY_TYPE_STRING, NULL, NULL, NULL, 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"color_primaries", PROPERTY_TYPE_STRING, NULL, "unspecified", "unspecified:bt_709:bt_601_625:bt_601_525:bt_2020",
     0, 1, ACCESS_TYPE_WRITE_INIT},
    {"transfer_characteristics", PROPERTY_TYPE_STRING, NULL, "unspecified",
     "unspecified:bt_709:bt_601_625:bt_601_525:smpte_st_2084:std_b67", 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"matrix_coefficients", PROPERTY_TYPE_STRING, NULL, "unspecified",
     "unspecified:bt_709:bt_601_625:bt_601_525:bt_2020", 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"chromaloc", PROPERTY_TYPE_INTEGER, NULL, "0", "0:5", 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"mastering_display_sei_x1", PROPERTY_TYPE_INTEGER, "First primary x.", "0", "0:50000", 0, 1,
     ACCESS_TYPE_WRITE_INIT},
    {"mastering_display_sei_y1", PROPERTY_TYPE_INTEGER, "First primary y.", "0", "0:50000", 0, 1,
     ACCESS_TYPE_WRITE_INIT},
    {"mastering_display_sei_x2", PROPERTY_TYPE_INTEGER, "Second primary x.", "0", "0:50000", 0, 1,
     ACCESS_TYPE_WRITE_INIT},
    {"mastering_display_sei_y2", PROPERTY_TYPE_INTEGER, "Second primary y.", "0", "0:50000", 0, 1,
     ACCESS_TYPE_WRITE_INIT},
    {"mastering_display_sei_x3", PROPERTY_TYPE_INTEGER, "Third primary x.", "0", "0:50000", 0, 1,
     ACCESS_TYPE_WRITE_INIT},
    {"mastering_display_sei_y3", PROPERTY_TYPE_INTEGER, "Third primary y.", "0", "0:50000", 0, 1,
     ACCESS_TYPE_WRITE_INIT},
    {"mastering_display_sei_wx", PROPERTY_TYPE_INTEGER, "White point x.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"mastering_display_sei_wy", PROPERTY_TYPE_INTEGER, "White point y.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"mastering_display_sei_max_lum", PROPERTY_TYPE_INTEGER, "Maximum display luminance.", "0", "0:2000000000", 0, 1,
     ACCESS_TYPE_WRITE_INIT},
    {"mastering_display_sei_min_lum", PROPERTY_TYPE_INTEGER, "Minimum display luminance.", "0", "0:2000000000", 0, 1,
     ACCESS_TYPE_WRITE_INIT},
    {"light_level_max_content", PROPERTY_TYPE_INTEGER, NULL, "0", "0:65535", 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"light_level_max_frame_average", PROPERTY_TYPE_INTEGER, NULL, "0", "0:65535", 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"force_slice_type", PROPERTY_TYPE_BOOLEAN, "Indicates that framework will try to force specific slice type.",
     "false", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT},
    {"uhd_bd", PROPERTY_TYPE_BOOLEAN, "Indicates UHD-BD encoding.", "false", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT},

    // Properties visible in the interface
    {"preset", PROPERTY_TYPE_STRING, "Encoder preset (initial state of encoder configuration).", "medium",
     "insanely_slow:ultra_slow:very_slow:slower:slow:medium:medium_plus:fast:faster:ultra_fast:insanely_fast", 0, 1,
     ACCESS_TYPE_USER},
    {"modifier", PROPERTY_TYPE_STRING,
     "One or more modifiers combined with '+' sign. Supported modifiers: low_delay, tune_psnr, realtime, cinema, "
     "bluray, hdr10, hlg, tune_vmaf, low_bitrate.",
     NULL, NULL, 0, 1, ACCESS_TYPE_USER},
    {"gop_intra_period", PROPERTY_TYPE_INTEGER, "GOP length.", "32", "0:65535", 0, 1, ACCESS_TYPE_USER},
    {"gop_idr_period", PROPERTY_TYPE_INTEGER, "Frequency of IDR frames / I frames.", "1", "0:65535", 0, 1,
     ACCESS_TYPE_USER},
    {"gop_min_intra_period", PROPERTY_TYPE_INTEGER, "Minimum distance between I-frames.", "4", "0:255", 0, 1,
     ACCESS_TYPE_USER},
    {"gop_minigop_size", PROPERTY_TYPE_INTEGER, "GOP structure (no. of B-frames + 1).", "8", "1:16", 0, 1,
     ACCESS_TYPE_USER},
    {"gop_min_minigop_size", PROPERTY_TYPE_INTEGER, "Minimal miniGOP size for adaptive miniGOP mode.", "8",
    "1:16", 0, 1, ACCESS_TYPE_USER},
    {"gop_max_refs", PROPERTY_TYPE_INTEGER,
     "Maximum reference frames that will be evaluated for inter-prediction.", "4", "1:8", 0, 1,
     ACCESS_TYPE_USER},
    {"me_scene_change", PROPERTY_TYPE_INTEGER, "Scene change detection sensitivity.", "0", "0:90", 0, 1,
     ACCESS_TYPE_USER},
    {"pps_cb_qp_offset", PROPERTY_TYPE_INTEGER, "Offset to the luma QP used for deriving Cb chroma QP.", "0", "-12:12",
     0, 1, ACCESS_TYPE_USER},
    {"pps_cr_qp_offset", PROPERTY_TYPE_INTEGER, "Offset to the luma QP used for deriving Cr chroma QP.", "0", "-12:12",
     0, 1, ACCESS_TYPE_USER},
    {"native_config_file", PROPERTY_TYPE_STRING, "File with encoder configuration in native Beamr syntax.", NULL, NULL,
     0, 1, ACCESS_TYPE_USER},
    {"param", PROPERTY_TYPE_STRING,
     "Native Beamr parameter using syntax \"name=value\". Use ':' separator to enter multiple parameters under single "
     "tag.",
     NULL, NULL, 0, 100, ACCESS_TYPE_USER},
    {"gop_structure_in_file", PROPERTY_TYPE_STRING, "File with GOP structure to force.", NULL, NULL, 0, 1,
     ACCESS_TYPE_USER},
    {"gop_structure_out_file", PROPERTY_TYPE_STRING, "File store encoded GOP structure.", NULL, NULL, 0, 1,
     ACCESS_TYPE_USER},
    {"debug_level", PROPERTY_TYPE_INTEGER, "0 - log errors, 1 - log encoder messages, 2 - debug trace to stderr.", "0",
     "0:2", 0, 1, ACCESS_TYPE_USER}};

static size_t getInfo(const PropertyInfo** info) {
    *info = propertyInfo;
    return sizeof(propertyInfo) / sizeof(PropertyInfo);
}

static size_t getSize() {
    return sizeof(PluginContext);
}

static Status init(HevcEncHandle handle, const HevcEncInitParams* init_params) {
    PluginContext* state = (PluginContext*)handle;
    if (state->ctrl)
        delete state->ctrl;
    state->ctrl = new PluginCtrl;
    if (state->encoder)
        delete state->encoder;
    state->encoder = new Encoder;

    try {
        const PropertyInfo* schema = propertyInfo;
        size_t count = sizeof(propertyInfo) / sizeof(PropertyInfo);

        for (int i = 0; i < (int)init_params->count; i++) {
            std::string name(init_params->properties[i].name);
            std::string value(init_params->properties[i].value);

            try {
                if ("plugin_path" == name) {
                    continue;
                }
                else if ("config_path" == name) {
                    state->ctrl->config_path = value;
                }
                else if ("temp_file" == name) {
                    state->ctrl->temp_file.push_back(value);
                }
                else if ("absolute_pass_num" == name) {
                    continue;
                }
                else if ("bit_depth" == name) {
                    state->ctrl->bit_depth = (uint8_t)parseInt(name, value, schema, count);
                }
                else if ("width" == name) {
                    state->ctrl->width = (uint16_t)parseInt(name, value, schema, count);
                }
                else if ("height" == name) {
                    state->ctrl->height = (uint16_t)parseInt(name, value, schema, count);
                }
                else if ("height" == name) {
                    state->ctrl->color_space = parseString(name, value, schema, count);
                }
                else if ("frame_rate" == name) {
                    state->ctrl->frame_rate = parseString(name, value, schema, count);
                }
                else if ("data_rate" == name) {
                    state->ctrl->data_rate = (unsigned int)parseInt(name, value, schema, count);
                }
                else if ("max_vbv_data_rate" == name) {
                    state->ctrl->max_vbv_data_rate = (unsigned int)parseInt(name, value, schema, count);
                }
                else if ("vbv_buffer_size" == name) {
                    state->ctrl->vbv_buffer_size = (unsigned int)parseInt(name, value, schema, count);
                }
                else if ("range" == name) {
                    state->ctrl->range = parseString(name, value, schema, count);
                }
                else if ("multi_pass" == name) {
                    state->ctrl->multi_pass = parseString(name, value, schema, count);
                }
                else if ("stats_file" == name) {
                    state->ctrl->stats_file = parseString(name, value, schema, count);
                }
                else if ("color_primaries" == name) {
                    state->ctrl->color_primaries = parseString(name, value, schema, count);
                }
                else if ("transfer_characteristics" == name) {
                    state->ctrl->transfer_characteristics = parseString(name, value, schema, count);
                }
                else if ("matrix_coefficients" == name) {
                    state->ctrl->matrix_coefficients = parseString(name, value, schema, count);
                }
                else if ("chromaloc" == name) {
                    state->ctrl->chromaloc = (int)parseInt(name, value, schema, count);
                }
                else if ("mastering_display_sei_x1" == name) {
                    state->ctrl->mastering_display_sei_x1 = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("mastering_display_sei_y1" == name) {
                    state->ctrl->mastering_display_sei_y1 = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("mastering_display_sei_x2" == name) {
                    state->ctrl->mastering_display_sei_x2 = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("mastering_display_sei_y2" == name) {
                    state->ctrl->mastering_display_sei_y2 = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("mastering_display_sei_x3" == name) {
                    state->ctrl->mastering_display_sei_x3 = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("mastering_display_sei_y3" == name) {
                    state->ctrl->mastering_display_sei_y3 = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("mastering_display_sei_wx" == name) {
                    state->ctrl->mastering_display_sei_wx = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("mastering_display_sei_wy" == name) {
                    state->ctrl->mastering_display_sei_wy = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("mastering_display_sei_max_lum" == name) {
                    state->ctrl->mastering_display_sei_max_lum = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("mastering_display_sei_min_lum" == name) {
                    state->ctrl->mastering_display_sei_min_lum = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("light_level_max_content" == name) {
                    state->ctrl->light_level_max_content = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("light_level_max_frame_average" == name) {
                    state->ctrl->light_level_max_frame_average = (int64_t)parseInt(name, value, schema, count);
                }
                else if ("force_slice_type" == name) {
                    // The fact we handle this property means that encoder can force slice type
                    state->ctrl->force_slice_type = parseBool(name, value, schema, count);
                }
                else if ("uhd_bd" == name) {
                    // The fact we handle this property means that encoder can encode UHD-BD
                    state->ctrl->uhd_bd = parseBool(name, value, schema, count);
                }
                else if ("preset" == name) {
                    state->ctrl->preset = parseString(name, value, schema, count);
                }
                else if ("modifier" == name) {
                    state->ctrl->modifier = parseStringList(
                        name, value, "+",
                        "low_delay:tune_psnr:realtime:cinema:bluray:hdr10:hlg:tune_vmaf:low_bitrate");
                }
                else if ("gop_intra_period" == name) {
                    state->ctrl->gop_intra_period = (uint16_t)parseInt(name, value, schema, count);
                }
                else if ("gop_idr_period" == name) {
                    state->ctrl->gop_idr_period = (uint16_t)parseInt(name, value, schema, count);
                }
                else if ("gop_min_intra_period" == name) {
                    state->ctrl->gop_min_intra_period = (uint8_t)parseInt(name, value, schema, count);
                }
                else if ("gop_minigop_size" == name) {
                    state->ctrl->gop_minigop_size = (uint8_t)parseInt(name, value, schema, count);
                }
                else if ("gop_min_minigop_size" == name) {
                    state->ctrl->gop_min_minigop_size = (uint8_t)parseInt(name, value, schema, count);
                }
                else if ("gop_max_refs" == name) {
                    state->ctrl->gop_max_refs = (uint8_t)parseInt(name, value, schema, count);
                }
                else if ("me_scene_change" == name) {
                    state->ctrl->me_scene_change = (uint8_t)parseInt(name, value, schema, count);
                }
                else if ("pps_cb_qp_offset" == name) {
                    state->ctrl->pps_cb_qp_offset = (uint32_t)parseInt(name, value, schema, count);
                }
                else if ("pps_cr_qp_offset" == name) {
                    state->ctrl->pps_cr_qp_offset = (uint32_t)parseInt(name, value, schema, count);
                }
                else if ("native_config_file" == name) {
                    state->ctrl->native_config_file = value;
                }
                else if ("param" == name) {
                    state->ctrl->param.push_back(value);
                }
                else if ("gop_structure_in_file" == name) {
                    state->ctrl->gop_structure_in_file = value;
                }
                else if ("gop_structure_out_file" == name) {
                    state->ctrl->gop_structure_out_file = value;
                }
                else if ("debug_level" == name) {
                    state->ctrl->debug_level = (int)parseInt(name, value, schema, count);
                }
                else if ("color_space" == name) {
                    continue; //hardcoded to i420 anyways
                }
                else {
                    state->ctrl->msg += "\nUnknown property: " + name;
                }
            }
            catch (std::exception& e) {
                state->ctrl->msg += "\n" + std::string(e.what());
            }
        }

        if (state->ctrl->msg.size()) {
            return STATUS_ERROR;
        }

        state->encoder->init(*state->ctrl);

        if (!state->ctrl->msg.empty()) {
            return STATUS_ERROR;
        }
        else {
            state->ctrl->msg += state->encoder->getVersion();
        }

        std::ifstream beamrCfg(state->ctrl->temp_file[0]);
        std::string line;
        while (std::getline(beamrCfg, line)) {
            state->ctrl->msg += "\n";
            state->ctrl->msg += line;
        }
        state->ctrl->msg += state->encoder->getMessage();
    }
    catch (std::exception& e) {
        state->ctrl->msg = std::string(e.what());
        auto debugMsg = state->encoder->getMessage();
        if (debugMsg.size())
            state->ctrl->msg = debugMsg + "\n" + state->ctrl->msg;
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

static Status close(HevcEncHandle handle) {
    PluginContext* state = (PluginContext*)handle;
    state->ctrl->msg.clear();
    try {
        if (state->encoder)
            state->encoder->close();

        if (state->ctrl)
            delete state->ctrl;
    }
    catch (std::exception& e) {
        state->ctrl->msg = std::string(e.what());
        auto debugMsg = state->encoder->getMessage();
        if (debugMsg.size())
            state->ctrl->msg = debugMsg + "\n" + state->ctrl->msg;
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

static Status
process(HevcEncHandle handle, const HevcEncPicture* picture, const size_t pictureNum, HevcEncOutput* output) {
    PluginContext* state = (PluginContext*)handle;
    state->ctrl->msg.clear();
    try {
        for (size_t i = 0; i < pictureNum; i++)
            state->encoder->feed(&picture[i]);

        state->encoder->getNal(output, state->ctrl->max_output_data);
        state->ctrl->msg = state->encoder->getMessage();
        return STATUS_OK;
    }
    catch (std::exception& e) {
        state->ctrl->msg = std::string(e.what());
        auto debugMsg = state->encoder->getMessage();
        if (debugMsg.size())
            state->ctrl->msg = debugMsg + "\n" + state->ctrl->msg;
        return STATUS_ERROR;
    }
}

static Status flush(HevcEncHandle handle, HevcEncOutput* output, int* isEmpty) {
    PluginContext* state = (PluginContext*)handle;
    state->ctrl->msg.clear();
    try {
        state->encoder->flush();
        state->encoder->getNal(output, state->ctrl->max_output_data);
        state->ctrl->msg = state->encoder->getMessage();
        *isEmpty = (output->nalNum == 0);
        return STATUS_OK;
    }
    catch (std::exception& e) {
        state->ctrl->msg = std::string(e.what());
        auto debugMsg = state->encoder->getMessage();
        if (debugMsg.size())
            state->ctrl->msg = debugMsg + "\n" + state->ctrl->msg;
        return STATUS_ERROR;
    }
}

static Status setProperty(HevcEncHandle handle, const Property* property) {
    PluginContext* state = (PluginContext*)handle;
    if (state->ctrl)
        state->ctrl->msg.clear();

    if (property->name == std::string("max_output_data")) {
        state->ctrl->max_output_data = atoi(property->value);
        return STATUS_OK;
    }

    return STATUS_ERROR;
}

static Status getProperty(HevcEncHandle handle, Property* property) {
    PluginContext* state = (PluginContext*)handle;
    if (state->ctrl)
        state->ctrl->msg.clear();

    if (NULL != property->name) {
        std::string name(property->name);
        if ("max_pass_num" == name) {
            strcpy(property->value, "2");
            return STATUS_OK;
        }
        else if ("temp_file_num" == name) {
            strcpy(property->value, "1");
            return STATUS_OK;
        }
    }
    return STATUS_ERROR;
}

static const char* getMessage(HevcEncHandle handle) {
    PluginContext* state = (PluginContext*)handle;
    return state->ctrl->msg.empty() ? NULL : state->ctrl->msg.c_str();
}

static HevcEncApi beamrPluginApi = {pluginName.c_str(), getInfo,     getSize,   init, close, process, flush,
                                    setProperty,        getProperty, getMessage};

DLB_EXPORT
HevcEncApi* hevcEncGetApi() {
    return &beamrPluginApi;
}

DLB_EXPORT
int hevcEncGetApiVersion() {
    return HEVC_ENC_API_VERSION;
}
