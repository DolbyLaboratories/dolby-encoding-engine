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

#include "scaling_flt_api.h"
#include <string>
#include <vector>

typedef struct
{
    scaling_flt_pic_format_t    format;
    size_t                      source_width;
    size_t                      source_height;
    size_t                      target_width;
    size_t                      target_height;
    scaling_flt_pic_t           source_pic;
    scaling_flt_pic_t           target_pic;
    std::string                 msg;
    int                         strength;
    std::vector<std::string>    temp_file;
} scaling_flt_example_data_t;

/* This structure can contain only pointers and simple types */
typedef struct
{
    scaling_flt_example_data_t* data;
} scaling_flt_example_t;

static
const
property_info_t scaling_base_info[] =
{
    {"temp_file_num", PROPERTY_TYPE_INTEGER, "Indicates how many temp files this plugin requires.", "3", NULL, 0, 1, ACCESS_TYPE_READ},
    { "temp_file", PROPERTY_TYPE_INTEGER, "Path to temp file.", NULL, NULL, 3, 3, ACCESS_TYPE_WRITE_INIT },
    { "format", PROPERTY_TYPE_INTEGER, "Format of input picture.", "0", "rgb_16:yuv420_10", 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "source_width", PROPERTY_TYPE_INTEGER, "Source picture width.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "source_height", PROPERTY_TYPE_INTEGER, "Source picture height.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "target_width", PROPERTY_TYPE_INTEGER, "Target picture width (0 = no width scaling).", "0", NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "target_height", PROPERTY_TYPE_INTEGER, "Target picture height (0 = no height scaling).", "0", NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
};

static
size_t
scaling_example_get_info
(const property_info_t** info    /**< [out] Pointer to array with property information */
)
{
    *info = scaling_base_info;
    return sizeof(scaling_base_info) / sizeof(property_info_t);
}

static
size_t
scaling_example_get_size()
{
    return sizeof(scaling_flt_example_t);
}

static
status_t
scaling_example_init
    (scaling_flt_handle_t               handle          /**< [in/out] filter instance handle */
    , const scaling_flt_init_params_t*   init_params     /**< [in] Properties to init filter instance */
    )
{
    scaling_flt_example_t* state = (scaling_flt_example_t*)handle;

    state->data = new scaling_flt_example_data_t;
    state->data->strength = 0;
    memset(&state->data->source_pic, 0, sizeof(scaling_flt_pic_t));
    memset(&state->data->target_pic, 0, sizeof(scaling_flt_pic_t));

    state->data->msg.clear();
    for (size_t i = 0; i < init_params->count; i++)
    {
        std::string name(init_params->property[i].name);
        std::string value(init_params->property[i].value);

        if ("format" == name)
        {
            if ("rgb_16" == value) state->data->format = SCALING_FLT_RGB_16;
            else if ("yuv420_10" == value) state->data->format = SCALING_FLT_YUV420_10;
            else return STATUS_ERROR;
        }
        else if ("source_width" == name)
        {
            state->data->source_width = atoi(value.c_str());
        }
        else if ("source_height" == name)
        {
            state->data->source_height = atoi(value.c_str());
        }
        else if ("target_width" == name)
        {
            state->data->target_width = atoi(value.c_str());
        }
        else if ("target_height" == name)
        {
            state->data->target_height = atoi(value.c_str());
        }
        else if ("temp_file" == name)
        {
            state->data->temp_file.push_back(value);
        }
    }

    if (!state->data->msg.empty())
    {
        std::string errmsg = "Parsing init params failed:" + state->data->msg;
        state->data->msg = errmsg;
        return STATUS_ERROR;
    }

    state->data->source_pic.width = state->data->source_width;
    state->data->source_pic.height = state->data->source_height;
    state->data->source_pic.buffer[0] = (void*)new short [state->data->source_pic.width*state->data->source_pic.height];
    state->data->source_pic.buffer[1] = (void*)new short [state->data->source_pic.width*state->data->source_pic.height];
    state->data->source_pic.buffer[2] = (void*)new short [state->data->source_pic.width*state->data->source_pic.height];

    state->data->target_pic.width = state->data->target_width;
    state->data->target_pic.height = state->data->target_height;
    state->data->target_pic.buffer[0] = (void*)new short [state->data->target_pic.width*state->data->target_pic.height];
    state->data->target_pic.buffer[1] = (void*)new short [state->data->target_pic.width*state->data->target_pic.height];
    state->data->target_pic.buffer[2] = (void*)new short [state->data->target_pic.width*state->data->target_pic.height];

    return STATUS_OK;
}

static
status_t
scaling_example_close
    (scaling_flt_handle_t handle   /**< [in/out] filter instance handle */
    )
{
    scaling_flt_example_t* state = (scaling_flt_example_t*)handle;

    if (state->data)
    {
        if (state->data->source_pic.buffer[0]) delete [] (short*) state->data->source_pic.buffer[0];
        if (state->data->source_pic.buffer[1]) delete [] (short*) state->data->source_pic.buffer[1];
        if (state->data->source_pic.buffer[2]) delete [] (short*) state->data->source_pic.buffer[2];
        if (state->data->target_pic.buffer[0]) delete [] (short*) state->data->target_pic.buffer[0];
        if (state->data->target_pic.buffer[1]) delete [] (short*) state->data->target_pic.buffer[1];
        if (state->data->target_pic.buffer[2]) delete [] (short*) state->data->target_pic.buffer[2];
        delete state->data;
    }
    return STATUS_OK;
}

static
status_t
scaling_example_process
    (scaling_flt_handle_t        handle   /**< [in/out] filter instance handle */
    , const scaling_flt_pic_t*    in      /**< [in] Input picture */
    , scaling_flt_pic_t*          out     /**< [out] Output picture */
    )
{
    scaling_flt_example_t* state = (scaling_flt_example_t*)handle;

    memcpy(state->data->source_pic.buffer[0], in->buffer[0], in->width*in->height*2);
    memcpy(state->data->source_pic.buffer[1], in->buffer[1], in->width*in->height*2);
    memcpy(state->data->source_pic.buffer[2], in->buffer[2], in->width*in->height*2);

    /*
    *  Add here some processing of state->data->source_pic.
    *  Check state->data->format to choose proper routine (RGB or YUV).
    *  Write scaled picture to state->data->target_pic.
    *  If you need temp files for some operations, use paths in state->data->temp_file vector.
    */

    out->width = state->data->target_pic.width;
    out->height = state->data->target_pic.height;
    out->buffer[0] = state->data->target_pic.buffer[0];
    out->buffer[1] = state->data->target_pic.buffer[1];
    out->buffer[2] = state->data->target_pic.buffer[2];

    return STATUS_OK;
}

static
status_t
scaling_example_set_property
    (scaling_flt_handle_t handle    /**< [in/out] filter instance handle */
    , const property_t* property     /**< [in] Property to write */
    )
{
    scaling_flt_example_t* state = (scaling_flt_example_t*)handle;
    if (NULL == state || NULL == property) return STATUS_ERROR;
    return STATUS_ERROR;
}

static
status_t
scaling_example_get_property
    (scaling_flt_handle_t handle    /**< [in/out] filter instance handle */
    , property_t* property           /**< [in/out] Property to read */
    )
{
    scaling_flt_example_t* state = (scaling_flt_example_t*)handle;
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
scaling_example_get_message
    (scaling_flt_handle_t handle        /**< [in/out] filter instance handle */
    )
{
    scaling_flt_example_t* state = (scaling_flt_example_t*)handle;
    return state->data->msg.c_str();
}

static scaling_flt_api_t plugin_api
{
    "example"
    , scaling_example_get_info
    , scaling_example_get_size
    , scaling_example_init
    , scaling_example_close
    , scaling_example_process
    , scaling_example_set_property
    , scaling_example_get_property
    , scaling_example_get_message
};

DLB_EXPORT
scaling_flt_api_t* scaling_flt_get_api()
{
    return &plugin_api;
}
