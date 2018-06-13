"""
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
"""
#!/usr/bin/env python
import sys
import json
import os

"""
SCRIPT FOR GENERATING COMMAND LINE INTERFACE INPUT FOR FFMPEG
THE SCRIPT CAN BE REPLACED BY USER WRITTEN ONE IN ANY PREFFERED LANGUAGE
THE SCRIPT IS CALLED FROM DEE APPLICATION WITH TWO ARGUMENTS - USER AND DEE'S JSON FILES WITH PARAMETERS FOR FFMPEG
IN FOLLOWING FORMAT: SCRIPT SYSTEM_JSON USER_JSON
SCRIPT GENERATES CLI INPUT FOR FFMPEG BASING ON PARAMETERS PROVIDED BY DEE AND USER
IF ANY USER PARAMS JSON IS NOT PROVIDED CLI INPUT IS GENERATED WITHOUT USER PARAMS
THE GENERATED CLI MUST BE SENDED BACK TO DEE BY PRINTING IT TO STANDARD OUTPUT PROCEDED BY "FFMPEG ENCODING CMD: " HEADER
EVERY MESSAGE PRINTED TO STANDARD OUTPUT WITHOUT HEADER WILL BE HANDLED AS ERROR AND PRINTED BY DEE
THIS BEHAVIOUR CAN BE SEEN IN "ROUTINE" SECTION

NOTE:
IN USER PARAMS JSON USE NATIVE X265 PARAMS NAMES
FOR BOOLEAN OPTIONS USE "True"/"False" VALUES. OTHER SYNTAX WILL BE PASSED DIRECTLY TO X265-PARAMS

EDIT THE CODE BELOW ON YOUR OWN RESPONSIBILITY
"""

# ROUTINE START
def generateCLIAndSendToHost(dee_cfg_json, user_cfg_json):
    cli = generateCLI(dee_cfg_json, user_cfg_json)
    sendToHost(cli)

def generateCLI(dee_cfg_json, user_cfg_json):
    [systemParameters, userParameters] = handleParameters(dee_cfg_json, user_cfg_json)
    cmd = createFfmpegCmd(systemParameters, userParameters)
    return cmd

def sendToHost(msg):
    msg = addHeader(msg)
    print(msg)

def addHeader(msg):
    return "FFMPEG ENCODING CMD: " + msg
# ROUTINE END

class UserParameters():
    separate_parameters = ['preset', 'forced-idr', 'tune']
    def __init__(self, user_cfg_json):
        self.parameters = dict()
        if user_cfg_json:
            cfg = parse_json_database(user_cfg_json)
            if 'user_config' not in cfg:
                print('{} key not found in user parameters file'.format("user_config"))
                exit(1)
            else:
                if 'x265' not in cfg['user_config']:
                    print('{} key not found in user parameters file'.format("x265"))
                    exit(1)
                else:
                    userParameters = cfg['user_config']['x265']

            for key, value in userParameters.items():
                key = key.replace("_", "-")
                if value == "False" or value == "True":
                    key, value = self.__handleBooleanParameters(key, value)
                self.parameters[key] = value

    def __handleBooleanParameters(self, key, value):
        if value == "False":
            if key[0:3] == "no-":
                key = key[3:]
            else:
                key = "no-" + key
        value = "1"
        return key, value

    def getParamValue(self, param_name):
        return self.parameters.get(param_name, "")

    def getParametersX265Formatted(self):
        x265_formatted_parameters = ""
        for parameter, value in self.parameters.items():
            if parameter in self.separate_parameters:
                continue
            x265_formatted_parameters += "{}={}:".format(parameter, value)

        x265_formatted_parameters = x265_formatted_parameters.strip(":")
        return x265_formatted_parameters

    def anyParameters(self):
        if not self.parameters:
            return False
        return True

def handleParameters(dee_cfg_json, user_cfg_json):
    userParameters = UserParameters(user_cfg_json)

    required_system_parameters = ['bit_depth', 'width', 'height',
                                  'color_space', 'frame_rate', 'data_rate',
                                  'max_vbv_data_rate', 'vbv_buffer_size',
                                  'ffmpeg_bin', 'input_file', 'output_file',
                                  'multipass', 'range', 'stats_file', 'color_description_present',
                                  'color_primaries', 'transfer_characteristics', 'matrix_coefficients',
                                  'light_level_information_sei_present', 'light_level_max_content',
                                  'mastering_display_sei_present',
                                  'mastering_display_sei_x1',
                                  'mastering_display_sei_y1',
                                  'mastering_display_sei_x2',
                                  'mastering_display_sei_y2',
                                  'mastering_display_sei_x3',
                                  'mastering_display_sei_y3',
                                  'mastering_display_sei_wx',
                                  'mastering_display_sei_wy',
                                  'mastering_display_sei_max_lum',
                                  'mastering_display_sei_min_lum']

    checkForbiddenParams(required_system_parameters, userParameters)

    cfg = parse_json_database(dee_cfg_json)
    if 'plugin_config' not in cfg:
        print('{} key not found in system parameters file'.format("plugin_config"))
        exit(1)
    else:
        systemParameters = cfg['plugin_config']
        for key in required_system_parameters:
            if key not in systemParameters:
                print('{} key not found in system parameters file'.format(key))
                exit(1)
        return (systemParameters, userParameters)

def checkForbiddenParams(required_system_parameters, userParameters):
    forbidden_params = ['input-csp', 'input-res', 'fps']
    for key in forbidden_params:
        if userParameters.getParamValue(key):
            print('{} is forbidden parameter and cannot be set. Provided in user parameter file'.format(key))
            exit(1)

    checkSystemConflict(required_system_parameters, userParameters)
    checkDvRequirementsConflict(userParameters)

def checkSystemConflict(required_system_parameters, userParameters):
    for key in required_system_parameters:
        key = key.replace("_", "-")
        if userParameters.getParamValue(key):
            print('{} is DEE\'s parameter and cannot be overridden. Provided in user parameter file'.format(key))
            exit(1)

def checkDvRequirementsConflict(userParameters):
    dv_required_params = ['aud', 'annexb', 'repeat-headers', 'hrd']
    for key in dv_required_params:
        if userParameters.getParamValue(key):
            print('{} is Dolby Vision\'s required parameter and cannot be overridden. Provided in user parameter file'.format(key))
            exit(1)
        elif userParameters.getParamValue("no-" + key):
            print('{} is Dolby Vision\'s required parameter and cannot be overridden. \"{}\" provided in user parameter file'.format(key, "no-" + key))
            exit(1)

def parse_json_database(json_database):
    if not os.path.isfile(json_database):
        print('Provided json database {} does not exists'.format(json_database))
        exit(1)
    json_data = open(json_database).read()
    return json.loads(json_data)

def createFfmpegCmd(systemParameters, userParameters):
    if systemParameters["multipass"] == "off":
        encoder_pass_param = ""
    else:
        encoder_pass_param = " -pass {num_pass} -passlogfile \"{stats_file}\" ".format(
            num_pass = "1" if systemParameters["multipass"] == "1st" else "2",
            stats_file = systemParameters["stats_file"])

    command_line = "{ffmpeg_bin} -f rawvideo -s {width}x{height} "                          \
                   "-pix_fmt {color_space}{bit_depth} -framerate {frame_rate} "             \
                   "-i {input_file} {x265config} {encoder_pass}"                            \
                   "-an -y -f hevc {output_file}".format(
                       ffmpeg_bin = systemParameters['ffmpeg_bin'],
                       width = systemParameters['width'],
                       height = systemParameters['height'],
                       color_space = systemParameters['color_space'],
                       bit_depth = "10le" if systemParameters['bit_depth'] == "10" else "",
                       frame_rate = systemParameters['frame_rate'],
                       input_file = systemParameters['input_file'],
                       x265config = generateX265Config(systemParameters, userParameters),
                       encoder_pass = encoder_pass_param,
                       output_file = systemParameters['output_file']
                   )

    return command_line

def generateX265Config(systemParameters, userParameters):

    separate_parameters= ""
    for separate_parameter in userParameters.separate_parameters:
        if userParameters.getParamValue(separate_parameter):
            separate_parameters += " -" + separate_parameter + " " + userParameters.getParamValue(separate_parameter)

    color_description_params = ""
    if systemParameters['color_description_present'] == "1":
        color_description_params = "colorprim={colorprim}:transfer={transfer}:"     \
                                   "colormatrix={colormatrix}:".format(
                                    colorprim = systemParameters['color_primaries'],
                                    transfer = systemParameters['transfer_characteristics'],
                                    colormatrix = systemParameters['matrix_coefficients']
                                    )

    light_level_params = ""
    if systemParameters['light_level_information_sei_present'] == "1":
        light_level_params = "max-cll={light_level_max_content},{light_level_max_frame_average}:".format(
                             light_level_max_content = systemParameters['light_level_max_content'],
                             light_level_max_frame_average = systemParameters['light_level_max_frame_average']
                             )

    mastering_display_params = ""
    if systemParameters['mastering_display_sei_present'] == "1":
        mastering_display_params = "master-display=G({sei_x1},{sei_y1})B({sei_x2},{sei_y2})"    \
                                   "R({sei_x3},{sei_y3})WP({sei_wx},{sei_wy})"                  \
                                   "L({max_lum},{min_lum}):".format(
                                   sei_x1 = systemParameters['mastering_display_sei_x1'],
                                   sei_y1 = systemParameters['mastering_display_sei_y1'],
                                   sei_x2 = systemParameters['mastering_display_sei_x2'],
                                   sei_y2 = systemParameters['mastering_display_sei_y2'],
                                   sei_x3 = systemParameters['mastering_display_sei_x3'],
                                   sei_y3 = systemParameters['mastering_display_sei_y3'],
                                   sei_wx = systemParameters['mastering_display_sei_wx'],
                                   sei_wy = systemParameters['mastering_display_sei_wy'],
                                   max_lum = systemParameters['mastering_display_sei_max_lum'],
                                   min_lum = systemParameters['mastering_display_sei_min_lum']
                                   )

    x265_cfg = "-c:v libx265{separate_parameters} -x265-params \"{color_description_params}{light_level_params}" \
               "{mastering_display_params}range={enc_range}:{x265_dv_requirements}:bitrate={data_rate}:"         \
               "vbv-maxrate={max_vbv_data_rate}:vbv-bufsize={vbv_buffer_size}{user_parameters}\"".format(
                separate_parameters = separate_parameters,
                color_description_params = color_description_params,
                light_level_params = light_level_params,
                mastering_display_params = mastering_display_params,
                enc_range = systemParameters['range'],
                x265_dv_requirements = getX265DvRequirements(),
                data_rate = systemParameters['data_rate'],
                max_vbv_data_rate = systemParameters['max_vbv_data_rate'],
                vbv_buffer_size = systemParameters['vbv_buffer_size'],
                user_parameters = ":" + userParameters.getParametersX265Formatted() if userParameters.anyParameters() else ""
                )

    return x265_cfg

def getX265DvRequirements():
    return "aud=1:annexb=1:repeat-headers=1:hrd=1:hash=1:chromaloc=2:sar=1"

if __name__ == "__main__":
    dee_cfg_json = sys.argv[1]
    user_cfg_json = None
    if len(sys.argv) > 2:
        user_cfg_json = sys.argv[2]
    generateCLIAndSendToHost(dee_cfg_json, user_cfg_json)
