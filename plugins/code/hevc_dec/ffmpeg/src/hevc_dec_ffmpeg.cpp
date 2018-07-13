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

#include "SystemCalls.h"
#include "hevc_dec_ffmpeg_utils.h"

#define TEMP_BUFFER (1024)
#define REQUIRED_TEMP_FILE_NUM (4)

#define PIPE_BUFFER_SIZE (1024*1024*64) // 64MB of buffer is enough to store an UHD 444 10bit frame
#define PIPE_TIMEOUT (600000)

static
const
property_info_t hevc_dec_ffmpeg_info[] =
{
      { "output_format", PROPERTY_TYPE_STRING, NULL, "any", "any:yuv420_10", 1, 1, ACCESS_TYPE_WRITE_INIT }
    , { "frame_rate", PROPERTY_TYPE_DECIMAL, NULL, "24", "23.976:24:25:29.97:30:48:50:59.94:60", 1, 1, ACCESS_TYPE_WRITE_INIT }
    , { "temp_file_num", PROPERTY_TYPE_INTEGER, "Indicates how many temp files this plugin requires.", "4", NULL, 0, 1, ACCESS_TYPE_READ }
    , { "temp_file", PROPERTY_TYPE_INTEGER, "Path to temp file.", NULL, NULL, 3, 3, ACCESS_TYPE_WRITE_INIT }

    // Only properties below (ACCESS_TYPE_USER) can be modified
    , { "ffmpeg_bin", PROPERTY_TYPE_STRING, "Path to ffmpeg binary.", "ffmpeg", NULL, 0, 1, ACCESS_TYPE_USER }
    , { "width", PROPERTY_TYPE_INTEGER, "Width of output in pixels.", "0", NULL, 1, 1, ACCESS_TYPE_USER }
    , { "height", PROPERTY_TYPE_INTEGER, "Height of output in pixels.", "0", NULL, 1, 1, ACCESS_TYPE_USER }
    , { "cmd_gen", PROPERTY_TYPE_STRING, "Path to a script that can generate an ffmpeg command line using the config file as input.", NULL, NULL, 1, 1, ACCESS_TYPE_USER }
    , { "interpreter", PROPERTY_TYPE_STRING, "Path to binary used to read the cmd_gen script.", "python", NULL, 0, 1, ACCESS_TYPE_USER }
    , { "redirect_stdout", PROPERTY_TYPE_BOOLEAN, "If set to true, terminal output from FFmpeg binary will be redirected to a temporary file.", "false", NULL, 0, 1, ACCESS_TYPE_USER }
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
    state->data->frame_rate_ext.frame_period = 24;
    state->data->frame_rate_ext.time_scale = 1;
    state->data->output_format = "any";
    state->data->decoded_picture.plane[0] = NULL;
    state->data->decoded_picture.plane[1] = NULL;
    state->data->decoded_picture.plane[2] = NULL;

#ifdef WIN32
    state->data->ffmpeg_bin = "ffmpeg.exe";
    state->data->interpreter = "python.exe";
#else
    state->data->ffmpeg_bin = "ffmpeg";
    state->data->interpreter = "python";
#endif

    state->data->ffmpeg_ret_code = 0;
    state->data->redirect_stdout = false;

    state->data->kill_ffmpeg = false;
    state->data->ffmpeg_running = false;
    state->data->piping_error = false;

    state->data->piping_mgr.setTimeout(PIPE_TIMEOUT);
    state->data->piping_mgr.setMaxbuf(PIPE_BUFFER_SIZE);

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
        else if ("redirect_stdout" == name)
        {
            if (value == "true")
            {
                state->data->redirect_stdout = true;
            }
            else if (value == "false")
            {
                state->data->redirect_stdout = false;
            }
            else
            {
                state->data->msg += "\nInvalid 'redirect_stdout' value.";
                continue;
            }
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

    if (!bin_exists(state->data->ffmpeg_bin, "-version", ""))
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

    state->data->in_pipe_id = state->data->piping_mgr.createNamedPipe(state->data->temp_file[0], INPUT_PIPE);
    state->data->out_pipe_id = state->data->piping_mgr.createNamedPipe(state->data->temp_file[1], OUTPUT_PIPE);

    if (state->data->in_pipe_id == -1 || state->data->out_pipe_id == -1)
    {
        state->data->msg = "Creating pipes failed.";
        return HEVC_DEC_ERROR;
    }

    if (state->data->piping_mgr.getPipePath(state->data->in_pipe_id, state->data->in_pipe_path) != PIPE_MGR_OK)
    {
        state->data->msg = "Creating pipes failed.";
        return HEVC_DEC_ERROR;
    }
    if (state->data->piping_mgr.getPipePath(state->data->out_pipe_id, state->data->out_pipe_path) != PIPE_MGR_OK)
    {
        state->data->msg = "Creating pipes failed.";
        return HEVC_DEC_ERROR;
    }

    if (state->data->width == 0 || state->data->height == 0)
    {
        state->data->msg = "Correct output width and height need to be specified.";
        return HEVC_DEC_ERROR;
    }

    init_picture_buffer(state->data);

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

    if (systemWithStdout(generate_cmd_call, ffmpeg_call) != SYSCALL_STATUS_OK)
    {
        state->data->msg = "Error building ffmpeg command line by calling: " + generate_cmd_call;
        return HEVC_DEC_ERROR;
    }

    if (!strip_header(ffmpeg_call))
    {
        state->data->msg = "cmd_gen script error: " + ffmpeg_call;
        return HEVC_DEC_ERROR;
    }

    strip_newline(ffmpeg_call);
    if (state->data->msg == "")
    {
        // LAUNCH FFMPEG //
        state->data->ffmpeg_running = true;
        state->data->ffmpeg_thread = std::thread(run_cmd_thread_func, ffmpeg_call, state->data);
        state->data->msg = "FFMPEG launched with the following command line: " + ffmpeg_call;
        if (state->data->redirect_stdout)
            state->data->msg += "\nFFMPEG log file: " + state->data->temp_file[3];
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

    bool plugin_failed = false;

    state->data->piping_mgr.close();
    if(state->data->ffmpeg_running == true)
    {
        state->data->kill_ffmpeg = true;
    }
    else
    {
        if (state->data->ffmpeg_ret_code != 0) plugin_failed = true;
    }

    if (state->data->piping_error)
    {
        state->data->kill_ffmpeg = true;
        plugin_failed = true;
    }
    if (state->data->ffmpeg_thread.joinable())
    {
        state->data->ffmpeg_thread.join();
    }

    if (state->data)
    {
        clean_picture_buffer(state->data);
        delete state->data;
    }

    return plugin_failed ? HEVC_DEC_ERROR : HEVC_DEC_OK;
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

    if (state->data->ffmpeg_ret_code != 0)
    {
        state->data->msg = "Ffmpeg runtime error.";
        state->data->msg += print_ffmpeg_state(state->data);
        state->data->msg += state->data->piping_mgr.printInternalState();
        return HEVC_DEC_ERROR;
    }

    if (write_to_ffmpeg(state->data, (void*)stream_buffer, buffer_size) == HEVC_DEC_ERROR)
    {
        state->data->msg += print_ffmpeg_state(state->data);
        state->data->msg += state->data->piping_mgr.printInternalState();
        return HEVC_DEC_ERROR;
    }

    hevc_dec_status_t reading_status = read_pic_from_ffmpeg(state->data);

    if (reading_status == HEVC_DEC_OK)
    {
        *output = state->data->decoded_picture;
    }

    return reading_status;
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

    if (flush_to_ffmpeg(state->data) == HEVC_DEC_PICTURE_NOT_READY)
    {
        // close input pipe to let FFMPEG know we finished sending input once the buffer is flushed
        state->data->piping_mgr.closePipe(state->data->in_pipe_id);
    }

    hevc_dec_status_t reading_status = read_pic_from_ffmpeg(state->data);

    if (reading_status == HEVC_DEC_OK)
    {
        *output = state->data->decoded_picture;
        *is_empty = false;
        return HEVC_DEC_OK;
    }
    else if (reading_status == HEVC_DEC_PICTURE_NOT_READY && state->data->ffmpeg_running)
    {
        *is_empty = false;
        return HEVC_DEC_PICTURE_NOT_READY;
    }
    else if (reading_status == HEVC_DEC_PICTURE_NOT_READY)
    {
        *is_empty = true;
        if (state->data->ffmpeg_ret_code != 0)
        {
            state->data->msg = "Ffmpeg runtime error.";
            state->data->msg += print_ffmpeg_state(state->data);
            state->data->msg += state->data->piping_mgr.printInternalState();
            return HEVC_DEC_ERROR;
        }
        return HEVC_DEC_PICTURE_NOT_READY;
    }
    else
    {
        state->data->msg += print_ffmpeg_state(state->data);
        state->data->msg += state->data->piping_mgr.printInternalState();
        return HEVC_DEC_ERROR;
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
