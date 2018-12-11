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

#include <string>
#include <cstdint>

#include "j2k_dec_comprimato.h"
#include "cmpto_j2k_dec.h"

#define MAX_PLANES (3)

#define CHECK_API_OK(statement) { \
    int result = statement; \
    if (result != CMPTO_OK) { \
        state->data->msg = "API error: " + std::string(cmpto_j2k_dec_get_last_error()); \
        return STATUS_ERROR; \
        }}

static
const
PropertyInfo j2k_dec_comprimato_info[] =
{
    { "plugin_path", PROPERTY_TYPE_STRING, "Path to this plugin.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "config_path", PROPERTY_TYPE_STRING, "Path to DEE config file.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "width", PROPERTY_TYPE_INTEGER, "Picture width", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "height", PROPERTY_TYPE_INTEGER, "Picture height", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT }
};

size_t
comprimato_get_info
(const PropertyInfo** info)
{
    *info = j2k_dec_comprimato_info;
    return sizeof(j2k_dec_comprimato_info) / sizeof(PropertyInfo);
}

typedef struct
{
    std::string         msg;
    size_t              width;
    size_t              height;
    cmpto_j2k_dec_ctx*  cmpto_ctx;
    cmpto_j2k_dec_cfg*  cmpto_cfg;
    uint16_t*           output_buffer;
    uint64_t            output_buffer_size;
} j2k_dec_comprimato_data_t;

/* This structure can contain only pointers and simple types */
typedef struct
{
    j2k_dec_comprimato_data_t* data;
} j2k_dec_comprimato_t;


size_t
comprimato_get_size()
{
    return sizeof(j2k_dec_comprimato_t);
}

Status
comprimato_init
(J2kDecHandle               handle          /**< [in/out] Decoder instance handle */
, const J2kDecInitParams*   init_params     /**< [in] Properties to init decoder instance */
)
{
    j2k_dec_comprimato_t* state = (j2k_dec_comprimato_t*)handle;

    state->data = new j2k_dec_comprimato_data_t;

    state->data->output_buffer = NULL;
    state->data->output_buffer_size = 0;
    state->data->width = 0;
    state->data->height = 0;
    state->data->msg.clear();

    for (size_t i = 0; i < init_params->count; i++)
    {
        std::string name(init_params->properties[i].name);
        std::string value(init_params->properties[i].value);

        if ("width" == name)
        {
            state->data->width = std::stoi(value);
        }
        else if ("height" == name)
        {
            state->data->height = std::stoi(value);
        }
        else if ("plugin_path" == name)
        {
            continue;
        }
        else if ("config_path" == name)
        {
            continue;
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

    cmpto_j2k_dec_ctx_create(NULL, &state->data->cmpto_ctx);
    cmpto_j2k_dec_cfg_create(state->data->cmpto_ctx, &state->data->cmpto_cfg);
    cmpto_j2k_dec_cfg_set_samples_format_type(state->data->cmpto_cfg, CMPTO_444_U16LE_P0P1P2);

    state->data->output_buffer = new uint16_t[state->data->width * state->data->height * MAX_PLANES];
    state->data->output_buffer_size = state->data->width * state->data->height * MAX_PLANES * sizeof(uint16_t);

    state->data->msg = "Initialized Comprimato decoder version " + std::string(cmpto_j2k_dec_get_version()->name);

    return STATUS_OK;
}

Status
comprimato_close
(J2kDecHandle handle
)
{
    j2k_dec_comprimato_t* state = (j2k_dec_comprimato_t*)handle;

    CHECK_API_OK(cmpto_j2k_dec_ctx_destroy(state->data->cmpto_ctx));
    delete[] state->data->output_buffer;
    delete state->data;

    return STATUS_OK;
}

Status
comprimato_process
(J2kDecHandle            handle  /**< [in/out] Decoder instance handle */
, const J2kDecInput*     in      /**< [in] Encoded input */
, J2kDecOutput*          out     /**< [out] Decoded output */
)
{
    j2k_dec_comprimato_t* state = (j2k_dec_comprimato_t*)handle;

    cmpto_j2k_dec_img* img;
    cmpto_j2k_dec_img* decoded_img;
    cmpto_j2k_dec_img_info decoded_img_info;
    int decoded_img_status;
    void* decoded_samples;
    size_t decoded_samples_size;

    // prepare decoding
    CHECK_API_OK(cmpto_j2k_dec_img_create(state->data->cmpto_ctx, &img));
    CHECK_API_OK(cmpto_j2k_dec_img_set_cstream(img, in->buffer, in->size, NULL));
    CHECK_API_OK(cmpto_j2k_dec_img_set_samples_buffer(img, (void*)state->data->output_buffer, state->data->output_buffer_size));

    // schedule decode
    CHECK_API_OK(cmpto_j2k_dec_img_decode(img, state->data->cmpto_cfg));

    // wait for image to be decoded
    CHECK_API_OK(cmpto_j2k_dec_ctx_get_decoded_img(state->data->cmpto_ctx, 1, &decoded_img, &decoded_img_status));
    if (CMPTO_J2K_DEC_IMG_OK != decoded_img_status) 
    {
        const char * decoding_error;
        cmpto_j2k_dec_img_get_error(img, &decoding_error);
        state->data->msg = "Image decoding failed: " + std::string(decoding_error);
        return STATUS_ERROR;
    }

    CHECK_API_OK(cmpto_j2k_dec_img_get_img_info(img, &decoded_img_info));
    CHECK_API_OK(cmpto_j2k_dec_img_get_samples(img, &decoded_samples, &decoded_samples_size));

    if (decoded_samples_size != state->data->output_buffer_size || (size_t)decoded_img_info.size_x != state->data->width || (size_t)decoded_img_info.size_y != state->data->height)
    {
        state->data->msg = "Decoded image size mismatch.";
        return STATUS_ERROR;
    }

    // fill j2k_dec_output_t
    out->width = decoded_img_info.size_x;
    out->height = decoded_img_info.size_y;
    out->buffer[0] = decoded_samples;
    out->buffer[1] = (uint16_t*)out->buffer[0] + decoded_img_info.size_x * decoded_img_info.size_y;
    out->buffer[2] = (uint16_t*)out->buffer[1] + decoded_img_info.size_x * decoded_img_info.size_y;

    CHECK_API_OK(cmpto_j2k_dec_img_destroy(img));

    return STATUS_OK;
}

Status
comprimato_get_property
(J2kDecHandle                /**< [in/out] Decoder instance handle */
, Property*                  /**< [in/out] Property to read */
)
{
    return STATUS_ERROR;
}

const char*
comprimato_get_message
(J2kDecHandle handle        /**< [in/out] Decoder instance handle */
)
{
    j2k_dec_comprimato_t* state = (j2k_dec_comprimato_t*)handle;
    return state->data->msg.empty() ? NULL : state->data->msg.c_str();
}
