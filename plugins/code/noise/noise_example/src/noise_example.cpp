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

#include "noise_flt_api.h"
#include <string>
#include <vector>

typedef struct
{
    size_t                      source_width;
    size_t                      source_height;
    noise_flt_pic_t             pic;
    std::string                 msg;
    std::string                 plugin_path;
    std::string                 config_path;
    int                         strength;
    std::vector<std::string>    temp_file;
} noise_flt_example_data_t;

/* This structure can contain only pointers and simple types */
typedef struct
{
    noise_flt_example_data_t* data;
} noise_flt_example_t;

static
const
property_info_t scaling_base_info[] =
{
    { "plugin_path", PROPERTY_TYPE_STRING, "Path to this plugin.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "config_path", PROPERTY_TYPE_STRING, "Path to DEE config file.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    {"temp_file_num", PROPERTY_TYPE_INTEGER, "Indicates how many temp files this plugin requires.", "3", NULL, 0, 1, ACCESS_TYPE_READ},
    { "temp_file", PROPERTY_TYPE_INTEGER, "Path to temp file.", NULL, NULL, 3, 3, ACCESS_TYPE_WRITE_INIT },
    { "source_width", PROPERTY_TYPE_INTEGER, "Source picture width.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "source_height", PROPERTY_TYPE_INTEGER, "Source picture height.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "strength", PROPERTY_TYPE_INTEGER, "Noise reduction strength.", "0", "0:20", 1, 1, ACCESS_TYPE_USER },
};

static
size_t
noise_example_get_info
(const property_info_t** info    /**< [out] Pointer to array with property information */
)
{
    *info = scaling_base_info;
    return sizeof(scaling_base_info) / sizeof(property_info_t);
}

static
size_t
noise_example_get_size()
{
    return sizeof(noise_flt_example_t);
}

static
status_t
noise_example_init
    (noise_flt_handle_t               handle          /**< [in/out] filter instance handle */
    , const noise_flt_init_params_t*   init_params     /**< [in] Properties to init filter instance */
    )
{
    noise_flt_example_t* state = (noise_flt_example_t*)handle;

    state->data = new noise_flt_example_data_t;
    state->data->strength = 0;
    memset(&state->data->pic, 0, sizeof(noise_flt_pic_t));

    state->data->msg.clear();
    state->data->plugin_path.clear();
    state->data->config_path.clear();
    for (size_t i = 0; i < init_params->count; i++)
    {
        std::string name(init_params->property[i].name);
        std::string value(init_params->property[i].value);

        if ("source_width" == name)
        {
            state->data->source_width = atoi(value.c_str());
        }
        else if ("source_height" == name)
        {
            state->data->source_height = atoi(value.c_str());
        }
        else if ("strength" == name)
        {
            state->data->strength = atoi(value.c_str());
            if (state->data->strength < 0 || state->data->strength > 20)
            {
                state->data->msg += "\nInvalid 'strength' value.";
            }
        }
        else if ("temp_file" == name)
        {
            state->data->temp_file.push_back(value);
        }
        else if ("plugin_path" == name)
        {
            state->data->plugin_path = value;
        }
        else if ("config_path" == name)
        {
            state->data->config_path = value;
        }
    }

    if (!state->data->msg.empty())
    {
        std::string errmsg = "Parsing init params failed:" + state->data->msg;
        state->data->msg = errmsg;
        return STATUS_ERROR;
    }

    state->data->pic.width = state->data->source_width;
    state->data->pic.height = state->data->source_height;
    state->data->pic.buffer[0] = (void*)new short [state->data->pic.width*state->data->pic.height];
    state->data->pic.buffer[1] = (void*)new short [state->data->pic.width*state->data->pic.height];
    state->data->pic.buffer[2] = (void*)new short [state->data->pic.width*state->data->pic.height];

    return STATUS_OK;
}

static
status_t
noise_example_close
    (noise_flt_handle_t handle   /**< [in/out] filter instance handle */
    )
{
    noise_flt_example_t* state = (noise_flt_example_t*)handle;

    if (state->data)
    {
        if (state->data->pic.buffer[0]) delete [] (short*)state->data->pic.buffer[0];
        if (state->data->pic.buffer[1]) delete [] (short*) state->data->pic.buffer[1];
        if (state->data->pic.buffer[2]) delete [] (short*) state->data->pic.buffer[2];
        delete state->data;
    }
    return STATUS_OK;
}

static
status_t
noise_example_process
    (noise_flt_handle_t        handle   /**< [in/out] filter instance handle */
    , const noise_flt_pic_t*    in      /**< [in] Input picture */
    , noise_flt_pic_t*          out     /**< [out] Output picture */
    )
{
    noise_flt_example_t* state = (noise_flt_example_t*)handle;

    memcpy(state->data->pic.buffer[0], in->buffer[0], in->width*in->height*2);
    memcpy(state->data->pic.buffer[1], in->buffer[1], in->width*in->height*2);
    memcpy(state->data->pic.buffer[2], in->buffer[2], in->width*in->height*2);

    /*
    *  Add here some processing of state->data->pic.
    *  If you need temp files for some operations, use paths in state->data->temp_file vector.
    */

    out->width = state->data->pic.width;
    out->height = state->data->pic.height;
    out->buffer[0] = state->data->pic.buffer[0];
    out->buffer[1] = state->data->pic.buffer[1];
    out->buffer[2] = state->data->pic.buffer[2];

    return STATUS_OK;
}

static
status_t
noise_example_set_property
    (noise_flt_handle_t handle    /**< [in/out] filter instance handle */
    , const property_t* property     /**< [in] Property to write */
    )
{
    noise_flt_example_t* state = (noise_flt_example_t*)handle;
    if (NULL == state || NULL == property) return STATUS_ERROR;
    return STATUS_ERROR;
}

static
status_t
noise_example_get_property
    (noise_flt_handle_t handle    /**< [in/out] filter instance handle */
    , property_t* property           /**< [in/out] Property to read */
    )
{
    noise_flt_example_t* state = (noise_flt_example_t*)handle;
    if (NULL == state) return STATUS_ERROR;
    
    if (NULL != property->name)
    {
        std::string name(property->name);
        if ("temp_file_num" == name)
        {
            strcpy(property->value, "3");
            return STATUS_OK;
        }
    }
    return STATUS_ERROR;
}

static
const char*
noise_example_get_message
    (noise_flt_handle_t handle        /**< [in/out] filter instance handle */
    )
{
    noise_flt_example_t* state = (noise_flt_example_t*)handle;
    return state->data->msg.c_str();
}

static noise_flt_api_t plugin_api
{
    "example"
    , noise_example_get_info
    , noise_example_get_size
    , noise_example_init
    , noise_example_close
    , noise_example_process
    , noise_example_set_property
    , noise_example_get_property
    , noise_example_get_message
};

DLB_EXPORT
noise_flt_api_t* noise_flt_get_api()
{
    return &plugin_api;
}
