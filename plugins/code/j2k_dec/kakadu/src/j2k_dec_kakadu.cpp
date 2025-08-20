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
#include "j2k_dec_kakadu.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <exception>

#ifdef _WIN32 // suppress windows kdu warnings
    #pragma warning(push)
    #pragma warning(disable: 4458) // declaration of '...' hides class member 
    #pragma warning(disable: 4100) // '...': unreferenced formal parameter 
#endif  //_WIN32
// Kakadu core includes
#include "kdu_elementary.h"
#include "kdu_messaging.h"
#include "kdu_params.h"
#include "kdu_sample_processing.h"
#include "kdu_compressed.h"

// Kakadu support includes
#include "kdu_stripe_decompressor.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif


using namespace kdu_supp;

#define MAX_PLANES (3)

static
const
PropertyInfo j2k_dec_kakadu_info[] =
{
    { "width", PROPERTY_TYPE_INTEGER, "Picture width", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "height", PROPERTY_TYPE_INTEGER, "Picture height", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT },
    { "thread_num", PROPERTY_TYPE_INTEGER, "Number of threads used for decoding. Value '0' disables multi-threading.", "8", "0:255", 0, 1, ACCESS_TYPE_USER }
};

size_t
kakadu_get_info
    (const PropertyInfo** info)
{
    *info = j2k_dec_kakadu_info;
    return sizeof(j2k_dec_kakadu_info) / sizeof(PropertyInfo);
}

struct j2k_dec_kakadu_data_t
{
    std::string         msg;
    size_t              width;
    size_t              height;
    int                 thread_num;
    kdu_int16*          output_buffer;
    short*              reorder_buffer;
    kdu_thread_env      env;
};

/* This structure can contain only pointers and simple types */
struct j2k_dec_kakadu_t
{
    j2k_dec_kakadu_data_t* data;
};


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
            if ((kdu_long)num_bytes > m_buffer_size-get_pos()) num_bytes = (int)(m_buffer_size-get_pos());
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
    data->thread_num = 8;
    data->msg.clear();
}

Status
kakadu_init
    (J2kDecHandle               handle          /**< [in/out] Decoder instance handle */
    ,const J2kDecInitParams*    init_params     /**< [in] Properties to init decoder instance */
    )
{
    j2k_dec_kakadu_t* state = (j2k_dec_kakadu_t*)handle;
    state->data = new j2k_dec_kakadu_data_t;
    
    init_data(state->data);
    for (size_t i = 0; i < init_params->count; i++)
    {
        std::string name(init_params->properties[i].name);
        std::string value(init_params->properties[i].value);

        try {
            if ("width" == name)
            {
                state->data->width = std::stoi(value);
            }
            else if ("height" == name)
            {
                state->data->height = std::stoi(value);
            }
            else if ("thread_num" == name)
            {
                state->data->thread_num = std::stoi(value);
            }
            else
            {
                state->data->msg += "Unknown property: " + name;
                return STATUS_ERROR;
            }
        } catch(std::exception&){
            state->data->msg += "Invalid '" + name + "' value: " + value;
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

    if (state->data->thread_num < 0 || state->data->thread_num > 255)
    {
        state->data->msg = "Invalid 'thread_num' value: " + std::to_string(state->data->thread_num);
        return STATUS_ERROR;
    }

    int buffer_size = (int)(state->data->width*state->data->height*MAX_PLANES);
    state->data->output_buffer = new kdu_int16[buffer_size];
    state->data->reorder_buffer = new short[buffer_size];

    if (state->data->thread_num) {
        state->data->env.create(); 
        for (int nt=1; nt < state->data->thread_num; nt++)
            if (!state->data->env.add_thread())
                state->data->thread_num = nt;
    }

    state->data->msg = "Initialized Kakadu j2k decoder version " + std::string(kdu_core::kdu_get_core_version());
    return STATUS_OK;
}

Status
kakadu_close
    (J2kDecHandle handle
    )
{
    j2k_dec_kakadu_t* state = (j2k_dec_kakadu_t*)handle;
    state->data->msg.clear();

    if (state->data)
    {
        if (state->data->thread_num)
            if (state->data->env.exists())
                state->data->env.destroy();

        if (state->data->output_buffer) delete [] state->data->output_buffer;
        if (state->data->reorder_buffer) delete [] state->data->reorder_buffer;
        if(state->data) {
            delete state->data;
            state->data = nullptr;
        }
    }

    return STATUS_OK;
}

static
void
prepare_output
    (j2k_dec_kakadu_t* state
    ,J2kDecOutput* out)
{
    out->width = state->data->width;
    out->height = state->data->height;
    size_t plane_bytes = out->width*out->height*2;
    out->buffer[0] = state->data->output_buffer;
    out->buffer[1] = (char*)out->buffer[0] + plane_bytes;
    out->buffer[2] = (char*)out->buffer[1] + plane_bytes;
}

Status
kakadu_process
    (J2kDecHandle           handle  /**< [in/out] Decoder instance handle */
    ,const J2kDecInput*     in      /**< [in] Encoded input */
    ,J2kDecOutput*          out     /**< [out] Decoded output */
    )
{
    j2k_dec_kakadu_t* state = (j2k_dec_kakadu_t*)handle;
    state->data->msg.clear();

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

    bool force_precise=true;
    bool want_fastest=false;
    kdu_stripe_decompressor decompressor;
    if (state->data->thread_num)
        decompressor.start(codestream, force_precise, want_fastest, &state->data->env);
    else
        decompressor.start(codestream, force_precise, want_fastest, NULL);

    int stripe_heights[3] = {dims0.size.y, dims1.size.y, dims2.size.y};
    int sample_offsets[3] = {0, dims0.size.x*dims0.size.y, dims0.size.x*dims0.size.y + dims1.size.x*dims1.size.y};
    int sample_gaps[3] = {1, 1, 1};
    decompressor.pull_stripe(state->data->output_buffer,stripe_heights, sample_offsets, sample_gaps, NULL, bit_depth, is_signed);
    decompressor.finish();
    codestream.destroy();
    prepare_output(state, out);

    return STATUS_OK;
}

Status
kakadu_get_property
    (J2kDecHandle                /**< [in/out] Decoder instance handle */
    , Property*                  /**< [in/out] Property to read */
    )
{
    return STATUS_ERROR;
}

const char*
kakadu_get_message
    (J2kDecHandle handle        /**< [in/out] Decoder instance handle */
    )
{
    j2k_dec_kakadu_t* state = (j2k_dec_kakadu_t*)handle;
    if (state && state->data)
        return state->data->msg.empty() ? NULL : state->data->msg.c_str();
    else
        return NULL;
}

