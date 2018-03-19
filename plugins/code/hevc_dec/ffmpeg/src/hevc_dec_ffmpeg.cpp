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

#include "hevc_dec_ffmpeg_utils.h"

#define MAX_BUFFERED_BLOBS 64
#define REQUIRED_TEMP_FILE_NUM 3

static
const
property_info_t hevc_dec_ffmpeg_info[] =
{
      { "output_format", PROPERTY_TYPE_STRING, NULL, "any", "any:yuv420_10", 1, 1, ACCESS_TYPE_WRITE_INIT }
    , { "frame_rate", PROPERTY_TYPE_DECIMAL, NULL, "24", "23.976:24:25:29.97:30:48:50:59.94:60", 1, 1, ACCESS_TYPE_WRITE_INIT }
    , { "temp_file_num", PROPERTY_TYPE_INTEGER, "Indicates how many temp files this plugin requires.", "2", NULL, 0, 1, ACCESS_TYPE_READ }
    , { "temp_file", PROPERTY_TYPE_INTEGER, "Path to temp file.", NULL, NULL, 3, 3, ACCESS_TYPE_WRITE_INIT }

    // Only properties below (ACCESS_TYPE_USER) can be modified
    , { "ffmpeg_bin", PROPERTY_TYPE_STRING, "Path to ffmpeg binary.", "ffmpeg", NULL, 0, 1, ACCESS_TYPE_USER }
    , { "width", PROPERTY_TYPE_INTEGER, "Width of output in pixels.", "0", NULL, 1, 1, ACCESS_TYPE_USER }
    , { "height", PROPERTY_TYPE_INTEGER, "Height of output in pixels.", "0", NULL, 1, 1, ACCESS_TYPE_USER }
    , { "cmd_gen", PROPERTY_TYPE_STRING, "Path to a script that can generate an ffmpeg command line using the config file as input.", NULL, NULL, 1, 1, ACCESS_TYPE_USER }
    , { "interpreter", PROPERTY_TYPE_STRING, "Path to binary used to read the cmd_gen script.", "python", NULL, 0, 1, ACCESS_TYPE_USER }
};


/* This structure can contain only pointers and simple types */
typedef struct
{
    hevc_dec_ffmpeg_data_t* data;
} hevc_dec_ffmpeg_t;


static
size_t
hevc_dec_ffmpeg_get_info
(const property_info_t** info
)
{
    *info = hevc_dec_ffmpeg_info;
    return sizeof(hevc_dec_ffmpeg_info) / sizeof(property_info_t);
}

static
size_t
hevc_dec_ffmpeg_get_size()
{
    return sizeof(hevc_dec_ffmpeg_t);
}

static
hevc_dec_status_t
hevc_dec_ffmpeg_init
(hevc_dec_handle_t handle
, const hevc_dec_init_params_t* init_params
)
{
    hevc_dec_ffmpeg_t* state = (hevc_dec_ffmpeg_t*)handle;
    state->data = new hevc_dec_ffmpeg_data_t;

    state->data->msg = "";
    state->data->width = 0;
    state->data->height = 0;
    state->data->output_bitdepth = 8;
    state->data->chroma_format = HEVC_DEC_COLOR_SPACE_I420;
    state->data->frame_rate.frame_period = 24000;
    state->data->frame_rate.time_scale = 1000;
    state->data->decoded_pictures_to_discard = 0;
    state->data->frame_rate_ext.frame_period = 24;
    state->data->frame_rate_ext.time_scale = 1;
    state->data->output_format = "any";

#ifdef WIN32
    state->data->ffmpeg_bin = "ffmpeg.exe";
    state->data->interpreter = "python.exe";
#else
    state->data->ffmpeg_bin = "ffmpeg";
    state->data->interpreter = "python";
#endif

    state->data->stop_writing_thread = false;
    state->data->stop_reading_thread = false;
    state->data->force_stop_writing_thread = false;
    state->data->force_stop_reading_thread = false;

    state->data->ffmpeg_ret_code = 0;

    state->data->missed_calls = 0;
    state->data->calls = 0;

    for (int i = 0; i < (int)init_params->count; i++)
    {
        std::string name(init_params->property[i].name);
        std::string value(init_params->property[i].value);

        if ("output_format" == name)
        {
            state->data->output_format = value;
            if (value == "any")
            {
                // by default we output yuv420
                state->data->chroma_format = HEVC_DEC_COLOR_SPACE_I420;
                state->data->output_bitdepth = 8;
            }
            else if (value == "yuv420_10")
            {
                state->data->chroma_format = HEVC_DEC_COLOR_SPACE_I420;
                state->data->output_bitdepth = 10;
            }
            else
            {
                state->data->msg += "\nUnsupported output format.";
            }
        }
        else if ("frame_rate" == name)
        {
            if (value != "23.976"
                &&  value != "24"
                &&  value != "25"
                &&  value != "29.97"
                &&  value != "30"
                &&  value != "48"
                &&  value != "50"
                &&  value != "59.94"
                &&  value != "60"
                )
            {
                state->data->msg += "\nInvalid 'frame_rate' value.";
                continue;
            }
            state->data->frame_rate_ext = string_to_fr(value);
        }
        else if ("ffmpeg_bin" == name)
        {
            state->data->ffmpeg_bin = value;
        }
        else if ("width" == name)
        {
            state->data->width = std::atoi(value.c_str());
        }
        else if ("height" == name)
        {
            state->data->height = std::atoi(value.c_str());
        }
        else if ("interpreter" == name)
        {
            state->data->interpreter = value;
        }
        else if ("cmd_gen" == name)
        {
            state->data->cmd_gen = value;
        }
        else if ("temp_file" == name)
        {
            state->data->temp_file.push_back(value);
        }
        else
        {
            state->data->msg += "\nUnknown XML property: " + name;
        }
    }

    if (state->data->temp_file.size() < REQUIRED_TEMP_FILE_NUM)
    {
        state->data->msg += "Need more temp files.";
    }
    if (state->data->ffmpeg_bin.empty())
    {
        state->data->msg += "Path to ffmpeg binary is not set.";
    }
    if (!bin_exists(state->data->ffmpeg_bin, "-version"))
    {
        state->data->msg += "Cannot access ffmpeg binary.";
    }
    if (state->data->interpreter.empty())
    {
        state->data->msg += "Path to interpreter binary is not set.";
    }

    if (state->data->msg != "")
    {
        return HEVC_DEC_ERROR;
    }

    if (state->data->in_pipe.createPipe(state->data->temp_file[0]) == -1)
    {
        state->data->msg = "Creating pipes failed.";
        return HEVC_DEC_ERROR;
    }
    if (state->data->out_pipe.createPipe(state->data->temp_file[1]) == -1)
    {
        state->data->msg = "Creating pipes failed.";
        return HEVC_DEC_ERROR;
    }
    if (state->data->width == 0 || state->data->height == 0)
    {
        state->data->msg = "Correct output width and height need to specified.";
        return HEVC_DEC_ERROR;
    }

    // build command line //

    std::string ffmpeg_call;

    if (write_cfg_file(state->data, state->data->temp_file[2]) == false)
    {
        state->data->msg = "Error writing encoding parameters to file: " + state->data->temp_file[2];
        return HEVC_DEC_ERROR;
    }

    if (state->data->cmd_gen.empty())
    {
        state->data->msg = "No command generation script provided. Use \'cmd_gen\' XML parameter.";
        return HEVC_DEC_ERROR;
    }

    std::string generate_cmd_call = "\"" + state->data->interpreter + "\" \"" + state->data->cmd_gen + "\" \"" + state->data->temp_file[2] + "\"";
    ffmpeg_call = run_cmd_get_output(generate_cmd_call);

    if (ffmpeg_call.empty())
    {
        state->data->msg = "Error building ffmpeg command line by calling: " + generate_cmd_call;
        return HEVC_DEC_ERROR;
    }

    if (state->data->msg == "")
    {
        // LAUNCH THREADS //

        state->data->ffmpeg_thread = std::thread(run_cmd_thread_func, ffmpeg_call, state->data);
        state->data->writer_thread = std::thread(writer_thread_func, state->data);
        state->data->reader_thread = std::thread(reader_thread_func, state->data);

        state->data->msg = "FFMPEG launched with the following command line: " + ffmpeg_call;
        return HEVC_DEC_OK;
    }
    else
    {
        return HEVC_DEC_ERROR;
    }
}

static
hevc_dec_status_t
hevc_dec_ffmpeg_close
(hevc_dec_handle_t handle
)
{
    hevc_dec_ffmpeg_t* state = (hevc_dec_ffmpeg_t*)handle;

    //double miss_rate = ((double)state->data->missed_calls/(double)state->data->calls) * 100.0;
    //std::cout << "Miss rate: " << miss_rate << " (" << state->data->missed_calls << " / " << state->data->calls << ")" << std::endl;

    if (state->data->ffmpeg_ret_code != 0)
    {
        state->data->force_stop_writing_thread = true;
        state->data->force_stop_reading_thread = true;
    }

    state->data->stop_writing_thread = true;
    state->data->stop_reading_thread = true;

    if (state->data->writer_thread.joinable())
    {
        state->data->writer_thread.join();
    }
    if (state->data->ffmpeg_thread.joinable())
    {
        state->data->ffmpeg_thread.join();
    }
    if (state->data->reader_thread.joinable())
    {
        state->data->reader_thread.join();
    }

    state->data->decoded_pictures_to_discard = state->data->decoded_pictures.size();
    remove_pictures(state->data);

    if (state->data) delete state->data;

    return HEVC_DEC_OK;
}

static
hevc_dec_status_t
hevc_dec_ffmpeg_process
(hevc_dec_handle_t           handle
, const void*                stream_buffer
, const size_t               buffer_size
, hevc_dec_picture_t*        output
)
{
    hevc_dec_ffmpeg_t* state = (hevc_dec_ffmpeg_t*)handle;
    remove_pictures(state->data);

    if (state->data->ffmpeg_ret_code != 0)
    {
        state->data->msg += "\nffmpeg runtime error.";
        return HEVC_DEC_ERROR;
    }

    // send new data to process by ffmpeg
    bool buffer_written_flag = false;
    while (state->data->ffmpeg_thread.joinable() && buffer_written_flag == false)
    {
        if (state->data->ffmpeg_ret_code != 0) break;
        state->data->in_buffer_mutex.lock();
        if (state->data->in_buffer.size() < MAX_BUFFERED_BLOBS)
        {
            state->data->in_buffer.push_back(new BufferBlob(stream_buffer, buffer_size));
            buffer_written_flag = true;
        }
        state->data->in_buffer_mutex.unlock();
    }

    // receive data from ffmpeg
    if (extract_pictures_from_buffer(state->data) != 0)
    {
        return HEVC_DEC_ERROR;
    }

    if (state->data->decoded_pictures.size() > 0)
    {
        *output = state->data->decoded_pictures.front();
        state->data->decoded_pictures_to_discard += 1;
        return HEVC_DEC_OK;
    }
    else
    {
        return HEVC_DEC_PICTURE_NOT_READY;
    }
}

static
hevc_dec_status_t
hevc_dec_ffmpeg_flush
(hevc_dec_handle_t      handle          /**< [in/out] Decoder instance handle */
, hevc_dec_picture_t*   output          /**< [in/out] Output buffer */
, int*                  is_empty        /**< [out] Flush indicator */
)
{
    hevc_dec_ffmpeg_t* state = (hevc_dec_ffmpeg_t*)handle;
    remove_pictures(state->data);

    if (state->data->ffmpeg_ret_code != 0)
    {
        state->data->msg += "\nffmpeg runtime error.";
        return HEVC_DEC_ERROR;
    }

    state->data->stop_writing_thread = true;
    if (state->data->writer_thread.joinable())
    {
        state->data->writer_thread.join();
    }
    if (state->data->ffmpeg_thread.joinable())
    {
        state->data->ffmpeg_thread.join();
    }
    state->data->stop_reading_thread = true;
    if (state->data->reader_thread.joinable())
    {
        state->data->reader_thread.join();
    }

    // flush must be called until no more picture are in the decoder buffer
    if (state->data->decoded_pictures.size() > 0 || state->data->out_buffer.size() > 0)
    {
        *is_empty = false;
    }
    else
    {
        *is_empty = true;
    }

    if (extract_pictures_from_buffer(state->data) != 0)
    {
        return HEVC_DEC_ERROR;
    }

    if (state->data->decoded_pictures.size() > 0)
    {
        *output = state->data->decoded_pictures.front();
        state->data->decoded_pictures_to_discard += 1;
        return HEVC_DEC_OK;
    }
    else
    {
        return HEVC_DEC_PICTURE_NOT_READY;
    }
}

static
hevc_dec_status_t
hevc_dec_ffmpeg_set_property
(hevc_dec_handle_t
,const property_t*
)
{
    return HEVC_DEC_ERROR;
}

static
hevc_dec_status_t
hevc_dec_ffmpeg_get_property
(hevc_dec_handle_t
,property_t* property
)
{
    if (NULL != property->name)
    {
        std::string name(property->name);
        if ("temp_file_num" == name)
        {
            strcpy(property->value, std::to_string(REQUIRED_TEMP_FILE_NUM).c_str());
            return HEVC_DEC_OK;
        }
    }

    return HEVC_DEC_ERROR;
}

static
const char*
hevc_dec_ffmpeg_get_message
(hevc_dec_handle_t handle
)
{
    hevc_dec_ffmpeg_t* state = (hevc_dec_ffmpeg_t*)handle;
    return state->data->msg.empty() ? NULL : state->data->msg.c_str();
}

static
hevc_dec_api_t ffmpeg_plugin_api =
{
    "ffmpeg"
    , hevc_dec_ffmpeg_get_info
    , hevc_dec_ffmpeg_get_size
    , hevc_dec_ffmpeg_init
    , hevc_dec_ffmpeg_close
    , hevc_dec_ffmpeg_process
    , hevc_dec_ffmpeg_flush
    , hevc_dec_ffmpeg_set_property
    , hevc_dec_ffmpeg_get_property
    , hevc_dec_ffmpeg_get_message
};

DLB_EXPORT
hevc_dec_api_t* hevc_dec_get_api()
{
    return &ffmpeg_plugin_api;
}
