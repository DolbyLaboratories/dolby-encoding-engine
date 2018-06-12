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
