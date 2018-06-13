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

"""
SCRIPT FOR GENERATING COMMAND LINE INPUT FOR FFMPEG
THE SCRIPT CAN BE REPLACED BY USER WRITTEN ONE IN ANY PREFFERED LANGUAGE
THE SCRIPT IS CALLED FROM DEE APPLICATION WITH ONE ARGUMENT - JSON FILE WITH PARAMETERS FOR FFMPEG CALLED BELOW "DEE_CFG_FILENAME"
SCRIPT GENERATES CLI FOR FFMPEG BASING ON PARAMETERS PROVIDED BY DEE AND USER BELOW
THE GENERATED CLI MUST BE SENDED BACK TO DEE BY PRINTING IT TO STANDARD OUTPUT PROCEDED BY "FFMPEG DECODING CMD: " HEADER
THIS BEHAVIOUR CAN BE SEEN IN "ROUTINE" SECTION
EVERY MESSAGE PRINTED TO STANDARD OUTPUT WITHOUT HEADER WILL BE HANDLED AS ERROR AND PRINTED BY DEE
"""

# EDIT THE CODE BELOW ON YOUR OWN RESPONSIBILITY
# ROUTINE START
def generateCLIAndSendToHost(dee_cfg_filename):
    cli = generateCLI(dee_cfg_filename)
    sendToHost(cli)

def generateCLI(dee_cfg_filename):
    systemParameters = handleParameters(dee_cfg_filename)
    cmd = createFfmpegCmd(systemParameters)
    return cmd

def sendToHost(msg):
    msg = addHeader(msg)
    print(msg)

def addHeader(msg):
    return "FFMPEG DECODING CMD: " + msg
# ROUTINE END

def handleParameters(dee_cfg_filename):
    required_system_parameters = ['output_bitdepth',
                                  'width',
                                  'height',
                                  'input_file',
                                  'output_file',
                                  'ffmpeg_bin']

    cfg = parse_json_database(dee_cfg_filename)
    if 'plugin_config' not in cfg:
        print('{} not found in system parameters file'.format("plugin_config"))
        exit(1)
    else:
        systemParameters = cfg['plugin_config']
        for key in required_system_parameters:
            if key not in systemParameters:
                print('{} not found in system parameters file'.format(key))
                exit(1)
        return systemParameters

def parse_json_database(json_database):
    json_data = open(json_database).read()
    return json.loads(json_data)

def createFfmpegCmd(systemParameters):
    command_line = "{ffmpeg_bin} -y -f hevc -i {input_file} -f rawvideo "   \
                   "-pix_fmt yuv420p{bit} -vf scale={width}:{height}:"    \
                   "force_original_aspect_ratio=decrease,pad={width}:"     \
                   "{height}:\(ow-iw\)/2:\(oh-ih\)/2 {output_file}".format( \
                   ffmpeg_bin = systemParameters['ffmpeg_bin'],
                   input_file = systemParameters['input_file'],
                   bit = "10le" if systemParameters['output_bitdepth'] != "8" else "",
                   width = systemParameters['width'],
                   height = systemParameters['height'],
                   output_file = systemParameters['output_file']
                   )

    return command_line

if __name__ == "__main__":
    dee_cfg_filename = sys.argv[1]
    generateCLIAndSendToHost(dee_cfg_filename)
