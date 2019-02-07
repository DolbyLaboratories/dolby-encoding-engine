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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>

#include "j2k_dec_api.h"
#include "GenericPlugin.h"
#include "SystemCalls.h"

#define MAX_PLANES (3)
static const int temp_file_num = 3;
static std::string temp_file_num_str = std::to_string(temp_file_num);
static int cur_instance_idx = 0;

static
const
PropertyInfo j2k_dec_ffmpeg_info[] =
{
    { "plugin_path", PROPERTY_TYPE_STRING, "Path to this plugin.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "config_path", PROPERTY_TYPE_STRING, "Path to DEE config file.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "temp_file_num", PROPERTY_TYPE_INTEGER, "Indicates how many temp files this plugin requires.", temp_file_num_str.c_str(), NULL, 0, 1, ACCESS_TYPE_READ},
    { "width", PROPERTY_TYPE_INTEGER, "Picture width", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "height", PROPERTY_TYPE_INTEGER, "Picture height", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "temp_file", PROPERTY_TYPE_INTEGER, "Path to temp file.", NULL, NULL, temp_file_num, temp_file_num, ACCESS_TYPE_WRITE_INIT },
    { "ffmpeg_bin", PROPERTY_TYPE_STRING, "Path to ffmpeg binary.", "ffmpeg", NULL, 0, 1, ACCESS_TYPE_USER },
    { "vcodec", PROPERTY_TYPE_STRING, "JPEG2000 decoder to be used via ffmpeg.", "libopenjpeg", NULL, 0, 1, ACCESS_TYPE_USER }
};

static
size_t
ffmpeg_get_info
    (const PropertyInfo** info)
{
    *info = j2k_dec_ffmpeg_info;
    return sizeof(j2k_dec_ffmpeg_info) / sizeof(PropertyInfo);
}

typedef struct
{
    std::string                 msg;
    std::string                 ffmpeg_bin;
    size_t                      width;
    size_t                      height;
    short*                      output_buffer;
    short*                      reorder_buffer;
    std::vector<std::string>    temp_file;
    std::string                 vcodec;
    int                         instance_idx;
    GenericPlugin               generic_plugin;
} j2k_dec_ffmpeg_data_t;

/* This structure can contain only pointers and simple types */
typedef struct
{
    j2k_dec_ffmpeg_data_t* data;
} j2k_dec_ffmpeg_t;

static
size_t
ffmpeg_get_size()
{
    return sizeof(j2k_dec_ffmpeg_t);
}

static
void
init_data
    (j2k_dec_ffmpeg_data_t* data)
{
    data->output_buffer = NULL;
    data->reorder_buffer = NULL;
    data->width = 0;
    data->height = 0;
    data->msg.clear();
    data->ffmpeg_bin = "ffmpeg";
    data->temp_file.clear();
    data->vcodec = "libopenjpeg";
    data->instance_idx = cur_instance_idx;
    cur_instance_idx++;
}

static bool
bin_exists(const std::string& bin, const std::string& arg, std::string& output)
{
    std::string cmd = "\"" + bin + "\" " + arg;

    int ret_code = 0;
    //int status = systemWithTimeout(cmd, ret_code, BINARY_CHECK_TIMEOUT, output);

    int status = systemWithStdout(cmd, output, ret_code);

    if (status != SYSCALL_STATUS_OK)
    {
        return false;
    }
    else
    {
        return (ret_code == 0);
    }
}

static
Status
ffmpeg_init
    (J2kDecHandle               handle          /**< [in/out] Decoder instance handle */
    ,const J2kDecInitParams*    init_params     /**< [in] Properties to init decoder instance */
    )
{
    j2k_dec_ffmpeg_t* state = (j2k_dec_ffmpeg_t*)handle;
    state->data = new j2k_dec_ffmpeg_data_t;
   
    init_data(state->data);

    for (size_t i = 0; i < init_params->count; i++)
    {
        std::string name(init_params->properties[i].name);
        std::string value(init_params->properties[i].value);

        if (state->data->generic_plugin.setProperty(&init_params->properties[i]) == STATUS_OK) continue;

        if  ("ffmpeg_bin" == name)
        {
            state->data->ffmpeg_bin = value;
        }
        else if ("temp_file" == name)
        {
            state->data->temp_file.push_back(value);
        }
        else if ("width" == name)
        {
            state->data->width = std::stoi(value);
        }
        else if ("height" == name)
        {
            state->data->height = std::stoi(value);
        }
        else if ("vcodec" == name)
        {
            state->data->vcodec = value;
        }
        else
        {
            state->data->msg += "\nUnknown XML property: " + name;
            return STATUS_ERROR;
        }
    }

    if (state->data->width <= 0)
    {
        state->data->msg = "Invalid 'width' value.";
        return STATUS_ERROR;
    }

    if (state->data->width <= 0)
    {
        state->data->msg = "Invalid 'width' value.";
        return STATUS_ERROR;
    }

    if ((int)state->data->temp_file.size() < temp_file_num)
    {
        state->data->msg = "Need more temp files.";
        return STATUS_ERROR;
    }

    if (state->data->ffmpeg_bin.empty())
    {
        state->data->msg = "Path to ffmpeg binary is not set.";
        return STATUS_ERROR;
    }

    std::string binCheckOutput = "";
    if (!bin_exists(state->data->ffmpeg_bin, "-version", binCheckOutput))
    {
        // cannot resolve binary path so try to expand it into an absolute path
        if (state->data->generic_plugin.expandPath(state->data->ffmpeg_bin) != STATUS_OK)
        {
            state->data->msg = "Cannot access ffmpeg binary.";
            return STATUS_ERROR;
        }
        else if (!bin_exists(state->data->ffmpeg_bin, "-version", binCheckOutput))
        {
            state->data->msg = "Cannot access ffmpeg binary.";
            return STATUS_ERROR;
        }
    }

    state->data->msg = "FFMPEG decoder version: " + binCheckOutput;

    size_t buffer_size = state->data->width*state->data->height*MAX_PLANES;
    state->data->output_buffer = new short[buffer_size];
    state->data->reorder_buffer = new short[buffer_size];

    return STATUS_OK;
}

static
Status
ffmpeg_close
    (J2kDecHandle handle
    )
{
    j2k_dec_ffmpeg_t* state = (j2k_dec_ffmpeg_t*)handle;

    cur_instance_idx = 0;

    if (state->data)
    {
        if (state->data->output_buffer) delete [] state->data->output_buffer;
        if (state->data->reorder_buffer) delete [] state->data->reorder_buffer;
        delete state->data;
    }

    return STATUS_OK;
}

static
void
prepare_output
    (j2k_dec_ffmpeg_t* state
    ,J2kDecOutput* out)
{
    int plane_samples_num = (int)(state->data->width*state->data->height);
    short* p_r = (short*)state->data->reorder_buffer;
    short* p_g = p_r + plane_samples_num;
    short* p_b = p_g + plane_samples_num;
    short* p_src = (short*)state->data->output_buffer;

    out->buffer[0] = (void*)p_r;
    out->buffer[1] = (void*)p_g;
    out->buffer[2] = (void*)p_b;

    for (size_t h = 0; h < state->data->height; h++)
    {
        for (size_t w = 0; w < state->data->width; w++)
        {
            *p_r++ = *p_src++;
            *p_g++ = *p_src++;
            *p_b++ = *p_src++;
        }
    }

    out->width = state->data->width;
    out->height = state->data->height;
}

static
Status
ffmpeg_process
    (J2kDecHandle           handle  /**< [in/out] Decoder instance handle */
    ,const J2kDecInput*     in      /**< [in] Encoded input */
    ,J2kDecOutput*          out     /**< [out] Decoded output */
    )
{
    j2k_dec_ffmpeg_t* state = (j2k_dec_ffmpeg_t*)handle;
    FILE* file_in;
    FILE* file_out;
    
    int offset = state->data->instance_idx*temp_file_num; // We want to access temp files for this instance specifically
    file_in = fopen(state->data->temp_file[0+offset].c_str(), "wb");
    if (NULL == file_in)
    {
        state->data->msg = "Could not open temp file 0.";
        return STATUS_ERROR;
    }
    fwrite(in->buffer, 1, in->size, file_in);
    fclose(file_in);
    
    std::string cmd = "\"" + state->data->ffmpeg_bin + "\"";
    cmd += " -vcodec " + state->data->vcodec;
    cmd += " -i \"" + state->data->temp_file[0+offset] + "\"";
    cmd += " -f rawvideo -pix_fmt rgb48le \"" + state->data->temp_file[1+offset] + "\"";
    cmd += " -y > \"" + state->data->temp_file[2 + offset] + "\"" + " 2>&1";
    
#ifdef WIN32
    // wrap command in extra quotations to ensure windows calls it properly
    cmd = "\"" + cmd + "\"";
#endif

    int rt = system(cmd.c_str());
    if (rt)
    {
        state->data->msg = "Command \"" + cmd + "\" returned code " + std::to_string(rt) + ".";
        std::ifstream ffmpeg_log;
        ffmpeg_log.open (state->data->temp_file[2+offset], std::ifstream::in);
        if (ffmpeg_log.is_open())
        {
            std::string line;
            while (std::getline(ffmpeg_log, line))
            {
                state->data->msg += "\n" + line;
            }
            ffmpeg_log.close();
        }
        
        return STATUS_ERROR;
    }

    file_out = fopen(state->data->temp_file[1+offset].c_str(), "rb");
    if (NULL == file_in)
    {
        state->data->msg = "Could not open temp file 1.";
        return STATUS_ERROR;
    }
    size_t samples_req = state->data->width*state->data->height*MAX_PLANES;
    size_t samples_read = fread(state->data->output_buffer, 2, samples_req, file_out);
    if (samples_req != samples_read)
    {
        state->data->msg = "Need more data in output buffer.";
        return STATUS_ERROR;
    }
    fclose(file_out);
    prepare_output(state, out);

    return STATUS_OK;
}

static
Status
ffmpeg_set_property
    (J2kDecHandle                     /**< [in/out] Decoder instance handle */
    , const Property*                 /**< [in] Property to write */
    )
{
    return STATUS_ERROR;
}

static
Status
ffmpeg_get_property
    (J2kDecHandle   handle                /**< [in/out] Decoder instance handle */
    , Property*       property            /**< [in/out] Property to read */
    )
{
    j2k_dec_ffmpeg_t* state = (j2k_dec_ffmpeg_t*)handle;
    if (NULL == state) return STATUS_ERROR;
    
    if (NULL != property->name)
    {
        std::string name(property->name);
        if ("temp_file_num" == name)
        {
            strcpy(property->value, std::to_string(temp_file_num).c_str());
            return STATUS_OK;
        }
    }
    return STATUS_ERROR;
}

static
const char*
ffmpeg_get_message
    (J2kDecHandle handle        /**< [in/out] Decoder instance handle */
    )
{
    j2k_dec_ffmpeg_t* state = (j2k_dec_ffmpeg_t*)handle;
    return state->data->msg.empty() ? NULL :state->data->msg.c_str();
}

static
J2kDecApi ffmpeg_plugin_api =
{
    "ffmpeg"
    ,ffmpeg_get_info
    ,ffmpeg_get_size
    ,ffmpeg_init
    ,ffmpeg_close
    ,ffmpeg_process
    ,ffmpeg_set_property
    ,ffmpeg_get_property
    ,ffmpeg_get_message
};

DLB_EXPORT
J2kDecApi* j2kDecGetApi()
{
    return &ffmpeg_plugin_api;
}

DLB_EXPORT
int j2kDecGetApiVersion(void)
{
    return J2K_DEC_API_VERSION;
}
