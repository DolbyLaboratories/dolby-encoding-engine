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

#include "hevc_dec_api.h"
#include "hevc_dec_beamr_impl.h"
#include "hevc_dec_beamr_utils.h"

static const std::string pluginName{"beamr"};

static const PropertyInfo propertyInfo[] = {
    {"plugin_path", PROPERTY_TYPE_STRING, "Path to this plugin.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT},
    {"config_path", PROPERTY_TYPE_STRING, "Path to DEE config file.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT},
    {"output_format", PROPERTY_TYPE_STRING, NULL, "any", "any:yuv420_10:rgb_16", 1, 1, ACCESS_TYPE_WRITE_INIT},
    {"frame_rate", PROPERTY_TYPE_DECIMAL, NULL, "24", "23.976:24:25:29.97:30:48:50:59.94:60", 1, 1,
    ACCESS_TYPE_WRITE_INIT},

    // Only properties below (ACCESS_TYPE_USER) can be modified
    {"use_sps_frame_rate", PROPERTY_TYPE_BOOLEAN,
     "Derive frame-rate from SPS. Use it only if you do not know actual frame rate.", "false", NULL, 0, 1,
     ACCESS_TYPE_USER},
    {"flags", PROPERTY_TYPE_INTEGER, "Bitmask with decoder flags (decimal uint32). Refer to Beamr SDK for flags' values.", "0",
     NULL, 0, 1, ACCESS_TYPE_USER},
    {"output_delay", PROPERTY_TYPE_INTEGER,
     "Reorder buffer size. Set 0 for immediate frame output w/o reordering, or it will be read from stream.", "16",
     "0:255", 0, 1, ACCESS_TYPE_USER},
    {"mt_disable", PROPERTY_TYPE_BOOLEAN, "Disable multi-threading.", "false", NULL, 0, 1, ACCESS_TYPE_USER},
    {"mt_num_threads", PROPERTY_TYPE_INTEGER, "Number of worker threads to run. 0=auto.", "4", "0:255", 0, 1,
     ACCESS_TYPE_USER},
    {"mt_num_wf_lines", PROPERTY_TYPE_INTEGER, "Number of lines in one group in parallel.", "4", "1:255", 0, 1,
     ACCESS_TYPE_USER},
    {"mt_num_frames", PROPERTY_TYPE_INTEGER, "Number of frames in parallel.", "3", "1:255", 0, 1, ACCESS_TYPE_USER},
    {"mt_num_input_units", PROPERTY_TYPE_INTEGER, "Number of slices in one frame processed in parallel.", "4", "1:255", 0, 1, ACCESS_TYPE_USER},
    {"mt_flags", PROPERTY_TYPE_INTEGER, "Bitmask with Multi-threading flags (decimal uint32). Refer to Beamr SDK for flags' values.", "3",
     NULL, 0, 1, ACCESS_TYPE_USER},
    {"mt_aff_mask", PROPERTY_TYPE_INTEGER, "Bitmask with CPU affinity flags (cores 0-63, decimal uint64). Refer to Beamr SDK for flags' values.", "0",
     NULL, 0, 1, ACCESS_TYPE_USER},
    {"disable_cpu_extensions", PROPERTY_TYPE_INTEGER, "Bitmask with CPU extensions to disable (decimal uint32). Refer to Beamr SDK for flags' values.", "0",
     NULL, 0, 1, ACCESS_TYPE_USER},
    {"debug_level", PROPERTY_TYPE_INTEGER, "0 - log errors, 1 - log decoder messages, 2 - debug trace to stderr.", "0", "0:2", 0, 1,
     ACCESS_TYPE_USER}};

static size_t getInfo(const PropertyInfo** info) {
    *info = propertyInfo;
    return sizeof(propertyInfo) / sizeof(PropertyInfo);
}

static size_t getSize() {
    return sizeof(PluginContext);
}

static HevcDecStatus init(HevcDecHandle handle, const HevcDecInitParams* init_params) {
    PluginContext* state = (PluginContext*)handle;
    state->ctrl = new PluginCtrl;
    state->decoder = new Decoder;

    state->ctrl->msg.clear();

    try {
        for (int i = 0; i < (int)init_params->count; i++) {
            std::string name(init_params->properties[i].name);
            std::string value(init_params->properties[i].value);

            try {
                if ("output_format" == name) {
                    // Value stored but doesn't affect processing. Decoder outputs native format.
                    state->ctrl->output_format = value;
                }
                else if ("frame_rate" == name) {
                    if (value != "23.976" && value != "24" && value != "25" && value != "29.97" && value != "30"
                        && value != "48" && value != "50" && value != "59.94" && value != "60") {
                        state->ctrl->msg += "\nInvalid 'frame_rate' value.";
                        continue;
                    }
                    state->ctrl->frame_rate = string2framerate(value);
                }
                else if ("plugin_path" == name) {
                    continue;
                }
                else if ("config_path" == name) {
                    continue;
                }
                else if ("use_sps_frame_rate" == name) {
                    state->ctrl->use_sps_frame_rate = string2bool(name, value);
                }
                else if ("flags" == name) {
                    state->ctrl->flags = (uint32_t)std::stoull(value);
                }
                else if ("output_delay" == name) {
                    state->ctrl->output_delay = string2int(name, value, 0, 255);
                }
                else if ("mt_disable" == name) {
                    state->ctrl->mt_disable = string2bool(name, value);
                }
                else if ("mt_num_threads" == name) {
                    state->ctrl->mt_num_threads = string2int(name, value, 0, 255);
                }
                else if ("mt_num_wf_lines" == name) {
                    state->ctrl->mt_num_wf_lines = string2int(name, value, 1, 255);
                }
                else if ("mt_num_frames" == name) {
                    state->ctrl->mt_num_frames = string2int(name, value, 1, 255);
                }
                else if ("mt_num_input_units" == name) {
                    state->ctrl->mt_num_input_units = string2int(name, value, 1, 255);
                }
                else if ("mt_flags" == name) {
                    state->ctrl->mt_flags = (uint32_t)std::stoull(value);
                }
                else if ("mt_aff_mask" == name) {
                    state->ctrl->mt_aff_mask = (uint64_t)std::stoull(value);
                }
                else if ("disable_cpu_extensions" == name) {
                    state->ctrl->disable_cpu_extensions = (uint32_t)std::stoull(value);
                }
                else if ("debug_level" == name) {
                    state->ctrl->debug_level = string2int(name, value, 0, 2);
                }
                else {
                    state->ctrl->msg += "\nUnknown XML property: " + name;
                }
            }
            catch (std::exception& e) {
                state->ctrl->msg += "\n" + std::string(e.what());
            }
        }

        if (state->ctrl->msg.size())
            return HEVC_DEC_ERROR;

        state->decoder->init(*state->ctrl);

        if (!state->ctrl->msg.empty()) {
            return HEVC_DEC_ERROR;
        }
        else {
            state->ctrl->msg += state->decoder->getVersion();
        }
    }
    catch (std::exception& e) {
        state->ctrl->msg = std::string(e.what());
        auto debugMsg = state->decoder->getMessage();
        if (debugMsg.size())
            state->ctrl->msg = debugMsg + "\n" + state->ctrl->msg;
        return HEVC_DEC_ERROR;
    }
    return HEVC_DEC_OK;
}

static HevcDecStatus close(HevcDecHandle handle) {
    PluginContext* state = (PluginContext*)handle;
    state->ctrl->msg.clear();
    try {
        if (state->decoder)
            state->decoder->close();

        if (state->ctrl)
            delete state->ctrl;
    }
    catch (std::exception& e) {
        state->ctrl->msg = std::string(e.what());
        auto debugMsg = state->decoder->getMessage();
        if (debugMsg.size())
            state->ctrl->msg = debugMsg + "\n" + state->ctrl->msg;
        return HEVC_DEC_ERROR;
    }
    return HEVC_DEC_OK;
}

static HevcDecStatus process(HevcDecHandle handle,
                             const void* streamBuffer,
                             const size_t bufferSize,
                             HevcDecPicture* output,
                             int* bufferConsumed) {
    PluginContext* state = (PluginContext*)handle;
    state->ctrl->msg.clear();
    try {
        state->decoder->feed((uint8_t*)streamBuffer, (uint64_t)bufferSize);
        *bufferConsumed = 1;
        bool picReady = state->decoder->getPic(state->ctrl->pic);
        auto retval = picReady ? HEVC_DEC_OK : HEVC_DEC_PICTURE_NOT_READY;

        if (picReady) {
            *output = state->ctrl->pic;
        }
        state->ctrl->msg = state->decoder->getMessage();
        return retval;
    }
    catch (std::exception& e) {
        state->ctrl->msg = std::string(e.what());
        auto debugMsg = state->decoder->getMessage();
        if (debugMsg.size())
            state->ctrl->msg = debugMsg + "\n" + state->ctrl->msg;
        return HEVC_DEC_ERROR;
    }
}

static HevcDecStatus flush(HevcDecHandle handle, HevcDecPicture* output, int* isEmpty) {
    PluginContext* state = (PluginContext*)handle;
    state->ctrl->msg.clear();
    try {
        state->decoder->flush();
        bool picReady = state->decoder->getPic(state->ctrl->pic);
        *isEmpty = picReady ? 0 : 1;
        if (picReady)
            *output = state->ctrl->pic;
        return picReady ? HEVC_DEC_OK : HEVC_DEC_PICTURE_NOT_READY;
    }
    catch (std::exception& e) {
        state->ctrl->msg = std::string(e.what());
        auto debugMsg = state->decoder->getMessage();
        if (debugMsg.size())
            state->ctrl->msg = debugMsg + "\n" + state->ctrl->msg;
        return HEVC_DEC_ERROR;
    }
}

static HevcDecStatus setProperty(HevcDecHandle, const Property*) {
    // There are no setters.
    return HEVC_DEC_ERROR;
}

static HevcDecStatus getProperty(HevcDecHandle, Property*) {
    // There are no getters.
    return HEVC_DEC_ERROR;
}

static const char* getMessage(HevcDecHandle handle) {
    PluginContext* state = (PluginContext*)handle;
    return state->ctrl->msg.empty() ? NULL : state->ctrl->msg.c_str();
}

static HevcDecApi beamrPluginApi = {pluginName.c_str(), getInfo,     getSize,   init, close, process, flush,
                                    setProperty,        getProperty, getMessage};

DLB_EXPORT
HevcDecApi* hevcDecGetApi() {
    return &beamrPluginApi;
}

DLB_EXPORT
int hevcDecGetApiVersion() {
    return HEVC_DEC_API_VERSION;
}
