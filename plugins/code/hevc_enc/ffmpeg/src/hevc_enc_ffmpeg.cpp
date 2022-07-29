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

#include "hevc_enc_api.h"
#include "hevc_enc_ffmpeg_utils.h"
#include <fstream>
#include <chrono>
#include <thread>

#define MAX_BUFFERED_BYTESTREAM 1024 * 1024 * 64 // na NAL should ever be larger than 64MB which is enough to store an UHD 444 10bit frame

static
const
PropertyInfo hevc_enc_ffmpeg_info[] =
{
    { "plugin_path", PROPERTY_TYPE_STRING, "Path to this plugin.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT }
    , { "config_path", PROPERTY_TYPE_STRING, "Path to DEE config file.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT }
    , { "max_pass_num", PROPERTY_TYPE_INTEGER, "Indicates how many passes encoder can perform (0 = unlimited).", "2", NULL, 0, 1, ACCESS_TYPE_READ }
    , { "max_output_data", PROPERTY_TYPE_INTEGER, "Limits number of output bytes (0 = unlimited).", "0", NULL, 0, 1, ACCESS_TYPE_WRITE }
    , { "bit_depth", PROPERTY_TYPE_STRING, NULL, NULL, "8:10", 1, 1, ACCESS_TYPE_WRITE_INIT }
    , { "width", PROPERTY_TYPE_INTEGER, NULL, NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT }
    , { "height", PROPERTY_TYPE_INTEGER, NULL, NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT }
    , { "color_space", PROPERTY_TYPE_STRING, NULL, "i420", "i400:i420:i422:i444", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "frame_rate", PROPERTY_TYPE_DECIMAL, NULL, NULL, "23.976:24:25:29.97:30:48:50:59.94:60:119.88:120", 1, 1, ACCESS_TYPE_WRITE_INIT }
    , { "data_rate", PROPERTY_TYPE_INTEGER, "Average data rate in kbps.", "15000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "max_vbv_data_rate", PROPERTY_TYPE_INTEGER, "Max VBV data rate in kbps.", "15000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "vbv_buffer_size", PROPERTY_TYPE_INTEGER, "VBV buffer size in kb.", "30000", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "range", PROPERTY_TYPE_STRING, NULL, "full", "limited:full", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "absolute_pass_num", PROPERTY_TYPE_INTEGER, NULL, "0", NULL, 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "stats_file", PROPERTY_TYPE_STRING, NULL, NULL, NULL, 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "multi_pass", PROPERTY_TYPE_STRING, NULL, "off", "off:1st:nth:last", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "temp_file_num", PROPERTY_TYPE_INTEGER, "Indicates how many temp files this plugin requires.", "4", NULL, 0, 1, ACCESS_TYPE_READ}
    , { "temp_file", PROPERTY_TYPE_INTEGER, "Path to temp file.", NULL, NULL, 3, 3, ACCESS_TYPE_WRITE_INIT }

    // generate_vui_param_videosignaltype
    , { "color_primaries", PROPERTY_TYPE_STRING, NULL, "unspecified", "unspecified:bt_709:bt_601_625:bt_601_525:bt_2020", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "transfer_characteristics", PROPERTY_TYPE_STRING, NULL, "unspecified", "unspecified:bt_709:bt_601_625:bt_601_525:smpte_st_2084:std_b67", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "matrix_coefficients", PROPERTY_TYPE_STRING, NULL, "unspecified", "unspecified:bt_709:bt_601_625:bt_601_525:bt_2020", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "chromaloc", PROPERTY_TYPE_INTEGER, NULL, "0", "0:5", 0, 1, ACCESS_TYPE_WRITE_INIT }

    // generate_mastering_display_color_volume_sei
    , { "mastering_display_sei_x1", PROPERTY_TYPE_INTEGER, "First primary x.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "mastering_display_sei_y1", PROPERTY_TYPE_INTEGER, "First primary y.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "mastering_display_sei_x2", PROPERTY_TYPE_INTEGER, "Second primary x.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "mastering_display_sei_y2", PROPERTY_TYPE_INTEGER, "Second primary y.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "mastering_display_sei_x3", PROPERTY_TYPE_INTEGER, "Third primary x.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "mastering_display_sei_y3", PROPERTY_TYPE_INTEGER, "Third primary y.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "mastering_display_sei_wx", PROPERTY_TYPE_INTEGER, "White point x.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "mastering_display_sei_wy", PROPERTY_TYPE_INTEGER, "White point y.", "0", "0:50000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "mastering_display_sei_max_lum", PROPERTY_TYPE_INTEGER, "Maximum display luminance.", "0", "0:2000000000", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "mastering_display_sei_min_lum", PROPERTY_TYPE_INTEGER, "Minimum display luminance.", "0", "0:2000000000", 0, 1, ACCESS_TYPE_WRITE_INIT }

    // generate_light_level_information_sei
    , { "light_level_max_content", PROPERTY_TYPE_INTEGER, NULL, "0", "0:65535", 0, 1, ACCESS_TYPE_WRITE_INIT }
    , { "light_level_max_frame_average", PROPERTY_TYPE_INTEGER, NULL, "0", "0:65535", 0, 1, ACCESS_TYPE_WRITE_INIT }

    // Only properties below (ACCESS_TYPE_USER) can be modified
    , { "ffmpeg_bin", PROPERTY_TYPE_STRING, "Path to ffmpeg binary.", "ffmpeg", NULL, 0, 1, ACCESS_TYPE_USER }
    , { "command_line", PROPERTY_TYPE_STRING, "Command line to be inserted into the ffmpeg command between the input and output specification.", NULL, NULL, 0, 100, ACCESS_TYPE_USER }
    , { "cmd_gen", PROPERTY_TYPE_STRING, "Path to a script that can generate an ffmpeg command line using the config file as input.", NULL, NULL, 1, 1, ACCESS_TYPE_USER }
    , { "user_params_file", PROPERTY_TYPE_STRING, "Path to a file with user's parameters. Path is passed to cmd gen script. If script provided with DEE is used it must be JSON database with following keys \"user_config\": \"x265\". For details see provided examples.", NULL, NULL, 0, 1, ACCESS_TYPE_USER }
    , { "interpreter", PROPERTY_TYPE_STRING, "Path to binary used to read the cmd_gen script.", "python", NULL, 0, 1, ACCESS_TYPE_USER }
    , { "redirect_stdout", PROPERTY_TYPE_BOOLEAN, "If set to true, terminal output from FFmpeg binary will be redirected to a temporary file.", "false", NULL, 0, 1, ACCESS_TYPE_USER }
};

static
size_t
ffmpeg_get_info
(const PropertyInfo** info
)
{
    *info = hevc_enc_ffmpeg_info;
    return sizeof(hevc_enc_ffmpeg_info) / sizeof(PropertyInfo);
}

static
size_t
ffmpeg_get_size()
{
    return sizeof(hevc_enc_ffmpeg_t);
}

static
Status
ffmpeg_init
(HevcEncHandle handle
, const HevcEncInitParams* init_params
)
{
    hevc_enc_ffmpeg_t* state = (hevc_enc_ffmpeg_t*)handle;
    state->data = new hevc_enc_ffmpeg_data_t;
    init_defaults(state);
    state->lib_initialized = false;
    state->data->msg.clear();

    if (!parse_init_params(state, init_params))
    {
        std::string errmsg = "Parsing init params failed:" + state->data->msg;
        state->data->msg = errmsg;
        return STATUS_ERROR;
    }

    if (0 == state->data->max_output_data)
    {
        state->data->max_output_data = SIZE_MAX;
    }


    state->data->in_pipe_id = state->data->piping_mgr.createNamedPipe(state->data->temp_file[0], INPUT_PIPE);
    state->data->out_pipe_id = state->data->piping_mgr.createNamedPipe(state->data->temp_file[1], OUTPUT_PIPE);

    if (state->data->in_pipe_id == -1 || state->data->out_pipe_id == -1)
    {
        state->data->msg = "Creating pipes failed.";
        return STATUS_ERROR;
    }

    if (state->data->piping_mgr.getPipePath(state->data->in_pipe_id, state->data->in_pipe_path) != PIPE_MGR_OK)
    {
        state->data->msg = "Creating pipes failed.";
        return STATUS_ERROR;
    }
    if (state->data->piping_mgr.getPipePath(state->data->out_pipe_id, state->data->out_pipe_path) != PIPE_MGR_OK)
    {
        state->data->msg = "Creating pipes failed.";
        return STATUS_ERROR;
    }

    std::string ffmpeg_call;

    if (state->data->command_line.size() > (unsigned int)state->data->pass_num && state->data->command_line[state->data->pass_num].empty() == false)
    {
        ffmpeg_call = "\"" + state->data->ffmpeg_bin + "\"";

        // input format
        ffmpeg_call += " -f rawvideo";

        // video size
        ffmpeg_call += " -s " + std::to_string(state->data->width) + "x" + std::to_string(state->data->height);

        // picture format
        ffmpeg_call += " -pix_fmt " + state->data->color_space;
        if (state->data->bit_depth == 10)
        {
            ffmpeg_call += "10le";
        }

        // frame rate
        ffmpeg_call += " -framerate " + fps_to_num_denom(state->data->frame_rate);

        // INPUT
        ffmpeg_call += " -i ";
        ffmpeg_call += "\"" + state->data->in_pipe_path + "\"";

        // user specified commandline
        ffmpeg_call += " " + state->data->command_line[state->data->pass_num];

        // disable audio
        ffmpeg_call += " -an";

        // overwrite file if present
        ffmpeg_call += " -y";

        // OUTPUT
        ffmpeg_call += " -f hevc ";
        ffmpeg_call += "\"" + state->data->out_pipe_path + "\"";
    }
    else
    {
        if (write_cfg_file(state->data, state->data->temp_file[2]) == false)
        {
            state->data->msg = "Error writing encoding parameters to file: " + state->data->temp_file[2];
            return STATUS_ERROR;
        }

        if (state->data->cmd_gen.empty())
        {
            state->data->msg = "No command line or command generation script provided. Use \'cmd_gen\' or \'command_line\' XML parameter.";
            return STATUS_ERROR;
        }

        if (state->data->user_params_file.empty())
        {
            state->data->msg = "No user parameters file provided. Use \'user_params_file\' XML parameter.";
            return STATUS_ERROR;
        }

        std::string generate_cmd_call = "\"" + state->data->interpreter + "\" \"" + state->data->cmd_gen + "\" \"" + state->data->temp_file[2] + "\" \"" + state->data->user_params_file + "\"";

        if (systemWithStdout(generate_cmd_call, ffmpeg_call) != SYSCALL_STATUS_OK)
        {
            state->data->msg = "Error building ffmpeg command line by calling: " + generate_cmd_call;
            return STATUS_ERROR;
        }
    }

    strip_newline(ffmpeg_call);

    if(!strip_header(ffmpeg_call))
    {
        state->data->msg = "cmd_gen script error: " + ffmpeg_call;
        return STATUS_ERROR;
    }

    if (state->data->msg.empty())
    {
        // LAUNCH THREAD //
        state->data->ffmpeg_running = true;
        state->data->ffmpeg_thread = std::thread(run_cmd_thread_func, ffmpeg_call, state->data);
        state->data->msg = "FFMPEG encoder version: " + state->data->version_string;
        state->data->msg += "\nFFMPEG launched with the following command line: " + ffmpeg_call;
        if (state->data->redirect_stdout)
            state->data->msg += "\nFFMPEG log file: " + state->data->temp_file[3];
        return STATUS_OK;
    }
    else
    {
        return STATUS_ERROR;
    }
}

static
Status
ffmpeg_close
(HevcEncHandle handle
)
{
    hevc_enc_ffmpeg_t* state = (hevc_enc_ffmpeg_t*)handle;

    clear_nalu_buffer(state->data);

    bool plugin_failed = false;
    
    state->data->piping_mgr.close();
    if(state->data->ffmpeg_running)
    {
        state->data->kill_ffmpeg = true;
    }
    else
    {
        if (state->data->ffmpeg_ret_code != 0) plugin_failed = true;
    }

    if (state->data->ffmpeg_thread.joinable())
    {
        state->data->ffmpeg_thread.join();
    }

    delete state->data;
    return plugin_failed ? STATUS_ERROR: STATUS_OK;
}

static
Status
ffmpeg_process
(HevcEncHandle          handle
, const HevcEncPicture*  picture
, const size_t               picture_num
, HevcEncOutput*         output
)
{
    hevc_enc_ffmpeg_t* state = (hevc_enc_ffmpeg_t*)handle;
    piping_status_t status;
    state->data->msg.clear();

    if (state->data->ffmpeg_ret_code != 0)
    {
        state->data->msg = "FFMPEG runtime error.";
        return STATUS_ERROR;
    }

    for (unsigned int i = 0; i < picture_num; i++)
    {
        HevcEncPicture current_pic = picture[i];

        int byte_num = current_pic.bitDepth == 8 ? 1 : 2;
        size_t plane_size[3];
        plane_size[0] = current_pic.width * current_pic.height * byte_num;
        plane_size[1] = plane_size[0];
        plane_size[2] = plane_size[0];

        if (current_pic.colorSpace == HEVC_ENC_COLOR_SPACE_I420)
        {
            plane_size[1] /= 4;
            plane_size[2] /= 4;
        }
        else if (current_pic.colorSpace == HEVC_ENC_COLOR_SPACE_I422)
        {
            plane_size[1] /= 2;
            plane_size[2] /= 2;
        }
        else if (current_pic.colorSpace != HEVC_ENC_COLOR_SPACE_I444)
        {
            state->data->msg = "YUV444 is not supported.";
            return STATUS_ERROR;
        }

        for (int j = 0; j < 3; j++)
        {
            bool plane_written_flag = false;
            size_t plane_data_written = 0;
            while (state->data->ffmpeg_running && plane_written_flag == false)
            {
                if (state->data->ffmpeg_ret_code != 0) break;

                size_t bytes_written = 0;
                status = state->data->piping_mgr.writeToPipe(state->data->in_pipe_id, (void*)((char*)current_pic.plane[j] + plane_data_written), plane_size[j] - plane_data_written, bytes_written);
                plane_data_written += bytes_written;
                if (status != PIPE_MGR_OK)
                {
                    state->data->msg = "Input pipe error " + std::to_string(status) + ".";
                    state->data->piping_error = true;
                    return STATUS_ERROR;
                }

                if (plane_data_written == plane_size[j])
                    plane_written_flag = true;
                
                if (bytes_written == 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    size_t bytes_read = 0;
    do
    {
        if (state->data->output_bytestream.size() >= MAX_BUFFERED_BYTESTREAM) break;

        status = state->data->piping_mgr.readFromPipe(state->data->out_pipe_id, state->data->output_temp_buf, READ_BUFFER_SIZE, bytes_read);
        if (status != PIPE_MGR_OK && status != PIPE_MGR_PIPE_CLOSED)
        {
            state->data->msg = "Output pipe error " + std::to_string(status) + ".";
            state->data->piping_error = true;
            return STATUS_ERROR;
        }
        if (bytes_read > 0)
        {
            state->data->output_bytestream.insert(state->data->output_bytestream.end(), state->data->output_temp_buf, state->data->output_temp_buf + bytes_read);
        }
    } 
    while (bytes_read > 0);

    clear_nalu_buffer(state->data);
    if (get_aud_from_bytestream(state->data->output_bytestream, state->data->nalus, false, state->data->max_output_data) == true)
    {
        output->nal = state->data->nalus.data();
        output->nalNum = state->data->nalus.size();
    }
    else
    {
        output->nal = NULL;
        output->nalNum = 0;
    }

    if (state->data->ffmpeg_ret_code)
    {
        state->data->msg += "FFMPEG exited with code " + std::to_string(state->data->ffmpeg_ret_code);
    }
    return (state->data->ffmpeg_ret_code != 0) ? STATUS_ERROR : STATUS_OK;
}

static
Status
ffmpeg_flush
(HevcEncHandle      handle
, HevcEncOutput*    output
, int*                  is_empty
)
{
    hevc_enc_ffmpeg_t* state = (hevc_enc_ffmpeg_t*)handle;
    piping_status_t status;
    state->data->msg.clear();

    // close input pipe to let FFMPEG know we finished sending input
    state->data->piping_mgr.closePipe(state->data->in_pipe_id);

    *is_empty = 0;

    size_t bytes_read = 0;
    do
    {
        if (state->data->output_bytestream.size() >= MAX_BUFFERED_BYTESTREAM) break;

        status = state->data->piping_mgr.readFromPipe(state->data->out_pipe_id, state->data->output_temp_buf, READ_BUFFER_SIZE, bytes_read);
        if (status != PIPE_MGR_OK && status != PIPE_MGR_PIPE_CLOSED)
        {
            state->data->msg = "output pipe error " + std::to_string(status) + ".";
            state->data->piping_error = true;
            return STATUS_ERROR;
        }
        if (bytes_read > 0)
        {
            state->data->output_bytestream.insert(state->data->output_bytestream.end(), state->data->output_temp_buf, state->data->output_temp_buf + bytes_read);
        }
    } 
    while (bytes_read > 0);

    clear_nalu_buffer(state->data);
    if (get_aud_from_bytestream(state->data->output_bytestream, state->data->nalus, true, state->data->max_output_data) == true)
    {
        output->nal = state->data->nalus.data();
        output->nalNum = state->data->nalus.size();
    }
    else if (state->data->ffmpeg_running)
    {
        output->nal = NULL;
        output->nalNum = 0;
        *is_empty = 0;
    }
    else
    {
        output->nal = NULL;
        output->nalNum = 0;
        *is_empty = 1;
    }

    if (state->data->ffmpeg_ret_code != 0)
    {
        state->data->msg = "FFMPEG runtime error.";
        *is_empty = 1;
        return STATUS_ERROR;
    }
    else
    {
        return STATUS_OK;
    }

}

static
Status
ffmpeg_set_property
(HevcEncHandle handle
, const Property* property
)
{
    hevc_enc_ffmpeg_t* state = (hevc_enc_ffmpeg_t*)handle;

    if (property->name == std::string("max_output_data"))
    {
        state->data->max_output_data = atoi(property->value);
        return STATUS_OK;
    }

    return STATUS_ERROR;
}

static
Status
ffmpeg_get_property
(HevcEncHandle handle
, Property* property
)
{
    hevc_enc_ffmpeg_t* state = (hevc_enc_ffmpeg_t*)handle;

    if (NULL != property->name)
    {
        std::string name(property->name);
        if ("max_pass_num" == name)
        {
            if (state->lib_initialized == true)
            {
                strcpy(property->value, std::to_string(state->data->max_pass_num).c_str());
            }
            else
            {
                strcpy(property->value, "2");
            }
            return STATUS_OK;
        }
        else if ("temp_file_num" == name)
        {
            strcpy(property->value, "4");
            return STATUS_OK;
        }
    }

    return STATUS_ERROR;
}

static
const char*
ffmpeg_get_message
(HevcEncHandle handle
)
{
    hevc_enc_ffmpeg_t* state = (hevc_enc_ffmpeg_t*)handle;
    return state->data->msg.empty() ? NULL : state->data->msg.c_str();
}

static
HevcEncApi ffmpeg_plugin_api =
{
    "ffmpeg"
    , ffmpeg_get_info
    , ffmpeg_get_size
    , ffmpeg_init
    , ffmpeg_close
    , ffmpeg_process
    , ffmpeg_flush
    , ffmpeg_set_property
    , ffmpeg_get_property
    , ffmpeg_get_message
};

DLB_EXPORT
HevcEncApi* hevcEncGetApi()
{
    return &ffmpeg_plugin_api;
}

DLB_EXPORT
int hevcEncGetApiVersion()
{
    return HEVC_ENC_API_VERSION;
}
