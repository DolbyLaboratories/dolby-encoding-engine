/*
* BSD 3-Clause License
*
* Copyright (c) 2018, Dolby Laboratories
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

#include "scaling_api.h"
#include <string>
#include <vector>

typedef struct
{
    ScalingPicFormat    	format;
    size_t                      source_width;
    size_t                      source_height;
    size_t                      target_width;
    size_t                      target_height;
    ScalingPic           	source_pic;
    ScalingPic           	target_pic;
    std::string                 msg;
    std::vector<std::string>    temp_file;
    bool                        allow_crop;

    std::string                 plugin_path;
    std::string                 config_path;

    int                 source_offset_top;
    int                 source_offset_bottom;
    int                 source_offset_left;
    int                 source_offset_right;

    int                 target_offset_top;
    int                 target_offset_bottom;
    int                 target_offset_left;
    int                 target_offset_right;
} scaling_example_data_t;

/* This structure can contain only pointers and simple types */
typedef struct
{
    scaling_example_data_t* data;
} scaling_example_t;

static
const
PropertyInfo scaling_base_info[] =
{
    { "plugin_path", PROPERTY_TYPE_STRING, "Path to this plugin.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "config_path", PROPERTY_TYPE_STRING, "Path to DEE config file.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    {"temp_file_num", PROPERTY_TYPE_INTEGER, "Indicates how many temp files this plugin requires.", "3", NULL, 0, 1, ACCESS_TYPE_READ},
    { "temp_file", PROPERTY_TYPE_INTEGER, "Path to temp file.", NULL, NULL, 3, 3, ACCESS_TYPE_WRITE_INIT },
    { "format", PROPERTY_TYPE_INTEGER, "Format of input picture.", "rgb_16", "rgb_16:yuv420_10", 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "source_width", PROPERTY_TYPE_INTEGER, "Source picture width.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "source_height", PROPERTY_TYPE_INTEGER, "Source picture height.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "target_width", PROPERTY_TYPE_INTEGER, "Target picture width (0 = no width scaling).", "0", NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "target_height", PROPERTY_TYPE_INTEGER, "Target picture height (0 = no height scaling).", "0", NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "source_offset_top", PROPERTY_TYPE_INTEGER, "Source picture active area top offset", "0", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT },
    { "source_offset_bottom", PROPERTY_TYPE_INTEGER, "Source picture active area bottom offset", "0", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT },
    { "source_offset_left", PROPERTY_TYPE_INTEGER, "Source picture active area left offset", "0", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT },
    { "source_offset_right", PROPERTY_TYPE_INTEGER, "Source picture active area right offset", "0", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT },
    { "allow_crop", PROPERTY_TYPE_BOOLEAN, "If set to false, no cropping or padding should occur.", "true", "true:false", 0, 1, ACCESS_TYPE_WRITE_INIT },
    { "get_target_offset_top", PROPERTY_TYPE_INTEGER, "Query target picture active area top offset", NULL, NULL, 1, 1, ACCESS_TYPE_READ },
    { "get_target_offset_bottom", PROPERTY_TYPE_INTEGER, "Query target picture active area bottom offset", NULL, NULL, 1, 1, ACCESS_TYPE_READ },
    { "get_target_offset_left", PROPERTY_TYPE_INTEGER, "Query target picture active area left offset", NULL, NULL, 1, 1, ACCESS_TYPE_READ },
    { "get_target_offset_right", PROPERTY_TYPE_INTEGER, "Query target picture active area right offset", NULL, NULL, 1, 1, ACCESS_TYPE_READ },
};

static
size_t
scaling_example_get_info
(const PropertyInfo** info    /**< [out] Pointer to array with property information */
)
{
    *info = scaling_base_info;
    return sizeof(scaling_base_info) / sizeof(PropertyInfo);
}

static
size_t
scaling_example_get_size()
{
    return sizeof(scaling_example_t);
}

static
void
calculate_offsets(ScalingHandle handle)
{
    scaling_example_t* state = (scaling_example_t*)handle;

    /*
    *  Assuming the picture is just stretched, the offsets will be stretched too.
    *  If you want to apply a different scaling mechanism, like introducing black bars,
    *  you need to adjust the below algorithm accordingly, to reflect the new active area.
    */

    double scale_factor_width = (double)(state->data->target_width) / (double)state->data->source_width;
    double scale_factor_height = (double)(state->data->target_height) / (double)state->data->source_height;

    state->data->target_offset_top = (int)((double)state->data->source_offset_top * scale_factor_height);
    state->data->target_offset_bottom = (int)((double)state->data->source_offset_bottom * scale_factor_height);
    state->data->target_offset_left = (int)((double)state->data->source_offset_left * scale_factor_width);
    state->data->target_offset_right = (int)((double)state->data->source_offset_right * scale_factor_width);
}

static
Status
scaling_example_init
    (ScalingHandle               handle          /**< [in/out] filter instance handle */
    , const ScalingInitParams*   init_params     /**< [in] Properties to init filter instance */
    )
{
    scaling_example_t* state = (scaling_example_t*)handle;

    state->data = new scaling_example_data_t;
    memset(&state->data->source_pic, 0, sizeof(ScalingPic));
    memset(&state->data->target_pic, 0, sizeof(ScalingPic));

    state->data->allow_crop = true;

    state->data->source_offset_top = 0;
    state->data->source_offset_bottom = 0;
    state->data->source_offset_left = 0;
    state->data->source_offset_right = 0;

    state->data->target_offset_top = 0;
    state->data->target_offset_bottom = 0;
    state->data->target_offset_left = 0;
    state->data->target_offset_right = 0;

    state->data->target_width = 0;     // 0 = same as source
    state->data->target_height = 0;    // 0 = same as source
    state->data->format = SCALING_RGB_16;

    state->data->plugin_path.clear();
    state->data->config_path.clear();

    state->data->msg.clear();
    for (size_t i = 0; i < init_params->count; i++)
    {
        std::string name(init_params->properties[i].name);
        std::string value(init_params->properties[i].value);

        if ("format" == name)
        {
            if ("rgb_16" == value) state->data->format = SCALING_RGB_16;
            else if ("yuv420_10" == value) state->data->format = SCALING_YUV420_10;
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
        else if ("source_offset_top" == name)
        {
            state->data->source_offset_top = atoi(value.c_str());
        }
        else if ("source_offset_bottom" == name)
        {
            state->data->source_offset_bottom = atoi(value.c_str());
        }
        else if ("source_offset_left" == name)
        {
            state->data->source_offset_left = atoi(value.c_str());
        }
        else if ("source_offset_right" == name)
        {
            state->data->source_offset_right = atoi(value.c_str());
        }
        else if ("allow_crop" == name)
        {
            if (value == "true")
            {
                state->data->allow_crop = true;
            }
            else if (value == "false")
            {
                state->data->allow_crop = false;
            }
            else
            {
                state->data->msg += "\nInvalid property value: " + value;
                return STATUS_ERROR;
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
            return STATUS_ERROR;
        }
    }

    if (!state->data->msg.empty())
    {
        std::string errmsg = "Parsing init params failed:" + state->data->msg;
        state->data->msg = errmsg;
        return STATUS_ERROR;
    }

    if (state->data->target_width == 0)
    {
        state->data->target_width = state->data->source_width;
    }
    if (state->data->target_height == 0)
    {
        state->data->target_height = state->data->source_height;
    }

    calculate_offsets(handle);

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
Status
scaling_example_close
    (ScalingHandle handle   /**< [in/out] filter instance handle */
    )
{
    scaling_example_t* state = (scaling_example_t*)handle;

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
Status
scaling_example_process
    (ScalingHandle         handle  /**< [in/out] filter instance handle */
    , const ScalingPic*    in      /**< [in] Input picture */
    , ScalingPic*          out     /**< [out] Output picture */
    )
{
    scaling_example_t* state = (scaling_example_t*)handle;

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
Status
scaling_example_set_property
    (ScalingHandle handle       /**< [in/out] filter instance handle */
    , const Property* property     /**< [in] Property to write */
    )
{
    scaling_example_t* state = (scaling_example_t*)handle;
    if (NULL == state || NULL == property) return STATUS_ERROR;
    return STATUS_ERROR;
}

static
Status
scaling_example_get_property
    (ScalingHandle handle       /**< [in/out] filter instance handle */
    , Property* property           /**< [in/out] Property to read */
    )
{
    scaling_example_t* state = (scaling_example_t*)handle;
    if (NULL == state) return STATUS_ERROR;

    std::string name(property->name);

    if ("temp_file_num" == name)
    {
        strcpy(property->value, "3");
        return STATUS_OK;
    }
    else if ("get_target_offset_top" == name && property->maxValueSz > std::to_string(state->data->target_offset_top).size())
    {
        strcpy(property->value, std::to_string(state->data->target_offset_top).c_str());
        return STATUS_OK;
    }
    else if ("get_target_offset_bottom" == name && property->maxValueSz > std::to_string(state->data->target_offset_bottom).size())
    {
        strcpy(property->value, std::to_string(state->data->target_offset_bottom).c_str());
        return STATUS_OK;
    }
    else if ("get_target_offset_left" == name && property->maxValueSz > std::to_string(state->data->target_offset_left).size())
    {
        strcpy(property->value, std::to_string(state->data->target_offset_left).c_str());
        return STATUS_OK;
    }
    else if ("get_target_offset_right" == name && property->maxValueSz > std::to_string(state->data->target_offset_right).size())
    {
        strcpy(property->value, std::to_string(state->data->target_offset_right).c_str());
        return STATUS_OK;
    }
    else
    {
        return STATUS_ERROR;
    }


    return STATUS_ERROR;
}

static
const char*
scaling_example_get_message
    (ScalingHandle handle        /**< [in/out] filter instance handle */
    )
{
    scaling_example_t* state = (scaling_example_t*)handle;
    return state->data->msg.c_str();
}

static ScalingApi plugin_api
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
ScalingApi* scalingGetApi()
{
    return &plugin_api;
}

DLB_EXPORT
int scalingGetApiVersion()
{
    return SCALING_API_VERSION;
}
