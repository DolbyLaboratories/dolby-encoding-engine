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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>

#include "j2k_dec_api.h"

// Kakadu core includes
#include "kdu_elementary.h"
#include "kdu_messaging.h"
#include "kdu_params.h"
#include "kdu_compressed.h"
#include "kdu_sample_processing.h"
// Kakadu support includes
#include "kdu_stripe_decompressor.h"
// Embed Kakadu support code
#include "avx2_stripe_transfer.cpp"
#include "kdu_stripe_decompressor.cpp"
#include "ssse3_stripe_transfer.cpp"

using namespace kdu_supp;

#define MAX_PLANES (3)

static
const
property_info_t j2k_dec_kakadu_info[] =
{
    { "width", PROPERTY_TYPE_INTEGER, "Picture width", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "height", PROPERTY_TYPE_INTEGER, "Picture height", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT }
};

static
size_t
kakadu_get_info
    (const property_info_t** info)
{
    *info = j2k_dec_kakadu_info;
    return sizeof(j2k_dec_kakadu_info) / sizeof(property_info_t);
}

typedef struct
{
    std::string         msg;
    size_t              width;
    size_t              height;
    kdu_int16*          output_buffer;
    short*              reorder_buffer;
} j2k_dec_kakadu_data_t;

/* This structure can contain only pointers and simple types */
typedef struct
{
    j2k_dec_kakadu_data_t* data;
} j2k_dec_kakadu_t;


class j2k_buffer : public kdu_compressed_source 
{
    public:
        j2k_buffer()
        { 
            m_buffer = NULL; 
            m_curpos = NULL;
            m_buffer_size = 0;
            m_capabilities = KDU_SOURCE_CAP_SEQUENTIAL | KDU_SOURCE_CAP_SEEKABLE;
        }

        ~j2k_buffer()
        { 
        }

        void open(void* buffer, kdu_long buffer_size) 
        { 
            m_buffer = (char*)buffer;
            m_buffer_size = buffer_size;
            m_curpos = m_buffer;
        }
    
        virtual int get_capabilities() { return m_capabilities; }
 
        virtual bool seek(kdu_long offset)
        {
            assert(m_buffer != NULL);
            assert(m_curpos != NULL);
            if (!(m_capabilities & KDU_SOURCE_CAP_SEEKABLE)) return false;
            m_curpos = m_buffer + offset;
            return true;
        }

        virtual kdu_long get_pos() { return (m_buffer==NULL)?-1:m_curpos-m_buffer; }

        virtual int read(kdu_byte *buf, int num_bytes)
        {
            assert(m_buffer != NULL);
            assert(m_curpos != NULL);
            if ((kdu_long)num_bytes > m_buffer_size-get_pos()) num_bytes = m_buffer_size-get_pos();
            memcpy(buf, m_curpos, num_bytes);
            m_curpos += num_bytes;
            return num_bytes;
        }


    private:
        int m_capabilities;
        kdu_long m_buffer_size;
        char* m_buffer;
        char* m_curpos;
};

static
size_t
kakadu_get_size()
{
    return sizeof(j2k_dec_kakadu_t);
}

static
void
init_data
    (j2k_dec_kakadu_data_t* data)
{
    data->output_buffer = NULL;
    data->reorder_buffer = NULL;
    data->width = 0;
    data->height = 0;
    data->msg.clear();
}

static
status_t
kakadu_init
    (j2k_dec_handle_t               handle          /**< [in/out] Decoder instance handle */
    ,const j2k_dec_init_params_t*   init_params     /**< [in] Properties to init decoder instance */
    )
{
    j2k_dec_kakadu_t* state = (j2k_dec_kakadu_t*)handle;
    state->data = new j2k_dec_kakadu_data_t;
   
    init_data(state->data);

    for (size_t i = 0; i < init_params->count; i++)
    {
        std::string name(init_params->property[i].name);
        std::string value(init_params->property[i].value);

        if ("width" == name)
        {
            state->data->width = std::stoi(value);
        }
        else if ("height" == name)
        {
            state->data->height = std::stoi(value);
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

    int buffer_size = state->data->width*state->data->height*MAX_PLANES;
    state->data->output_buffer = new kdu_int16[buffer_size];
    state->data->reorder_buffer = new short[buffer_size];

    return STATUS_OK;
}

static
status_t
kakadu_close
    (j2k_dec_handle_t handle
    )
{
    j2k_dec_kakadu_t* state = (j2k_dec_kakadu_t*)handle;

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
    (j2k_dec_kakadu_t* state
    ,j2k_dec_output_t* out)
{
    int plane_samples_num = state->data->width*state->data->height;
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
status_t
kakadu_process
    (j2k_dec_handle_t           handle  /**< [in/out] Decoder instance handle */
    ,const j2k_dec_input_t*     in      /**< [in] Encoded input */
    ,j2k_dec_output_t*          out     /**< [out] Decoded output */
    )
{
    j2k_dec_kakadu_t* state = (j2k_dec_kakadu_t*)handle;
    
    j2k_buffer input;
    input.open(in->buffer, (kdu_long)in->size);
    kdu_codestream codestream; codestream.create(&input);
    codestream.set_fussy();

    int num_components = codestream.get_num_components();
    if (num_components != 3)
    {
        state->data->msg = "Picture must consist of 3 components.";
        return STATUS_ERROR;
    }

    codestream.apply_input_restrictions(0,num_components,0,0,NULL);

    kdu_dims dims0, dims1, dims2;
    codestream.get_dims(0,dims0);
    codestream.get_dims(1,dims1);
    codestream.get_dims(2,dims2);

    if (dims0 != dims1 || dims0 != dims2)
    {
        state->data->msg = "Mismatching dimensions.";
        return STATUS_ERROR;
    }

    int bit_depth[] = {16, 16, 16};
    bool is_signed[] = {false, false, false};
    kdu_stripe_decompressor decompressor;
    decompressor.start(codestream);
    int stripe_heights[3] = {dims0.size.y, dims0.size.y, dims0.size.y};
    
    decompressor.pull_stripe(state->data->output_buffer,stripe_heights, NULL, NULL, NULL, bit_depth, is_signed);
    
    decompressor.finish();
    codestream.destroy();
    prepare_output(state, out);

    return STATUS_OK;
}

static
status_t
kakadu_set_property
    (j2k_dec_handle_t   handle          /**< [in/out] Decoder instance handle */
    , const property_t* property        /**< [in] Property to write */
    )
{
    return STATUS_ERROR;
}

static
status_t
kakadu_get_property
    (j2k_dec_handle_t   handle              /**< [in/out] Decoder instance handle */
    , property_t*       property            /**< [in/out] Property to read */
    )
{
    return STATUS_ERROR;
}

static
const char*
kakadu_get_message
    (j2k_dec_handle_t handle        /**< [in/out] Decoder instance handle */
    )
{
    j2k_dec_kakadu_t* state = (j2k_dec_kakadu_t*)handle;
    return state->data->msg.empty() ? NULL :state->data->msg.c_str();
}

static
j2k_dec_api_t kakadu_plugin_api =
{
    "kakadu"
    ,kakadu_get_info
    ,kakadu_get_size
    ,kakadu_init
    ,kakadu_close
    ,kakadu_process
    ,kakadu_set_property
    ,kakadu_get_property
    ,kakadu_get_message
};

DLB_EXPORT
j2k_dec_api_t* j2k_dec_get_api()
{
    return &kakadu_plugin_api;
}