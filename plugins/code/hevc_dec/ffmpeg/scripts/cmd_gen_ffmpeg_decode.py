#!/usr/bin/env python
import sys

file_name = sys.argv[1]

with open(file_name) as file:
    content = file.readlines()

content = [x.strip() for x in content]

bit_depth = "8"
width = "0"
height = "0"
color_space = "yuv420p"
frame_rate = "24"
data_rate = "0"
max_vbv_data_rate = "0"
vbv_buffer_size = "0"
ffmpeg_bin = "ffmpeg.exe" 
input_file = "input_file"
output_file = "output_file"
multipass = "off"
stats_file = ""
color_description_present = 0
color_primaries = 0
transfer_characteristics = 0
matrix_coefficients = 0
light_level_information_sei_present = 0
light_level_max_content = 0
light_level_max_frame_average = 0
mastering_display_sei_present = 0
mastering_display_sei_x1 = 0
mastering_display_sei_y1 = 0
mastering_display_sei_x2 = 0
mastering_display_sei_y2 = 0
mastering_display_sei_x3 = 0
mastering_display_sei_y3 = 0
mastering_display_sei_wx = 0
mastering_display_sei_wy = 0
mastering_display_sei_max_lum = 0
mastering_display_sei_min_lum = 0

for x in content:
    x.strip()
    split_line = x.split("=", 1)
    
    if split_line[0] == "bit_depth":
        bit_depth = split_line[1]
    elif split_line[0] == "width":
        width = split_line[1]
    elif split_line[0] == "height":
        height = split_line[1]
    elif split_line[0] == "color_space":
        color_space = split_line[1]
    elif split_line[0] == "frame_rate":
        frame_rate = split_line[1]
    elif split_line[0] == "data_rate":
        data_rate = split_line[1]
    elif split_line[0] == "max_vbv_data_rate":
        max_vbv_data_rate = split_line[1]
    elif split_line[0] == "vbv_buffer_size":
        vbv_buffer_size = split_line[1]
    elif split_line[0] == "input_file":
        input_file = split_line[1]
    elif split_line[0] == "output_file":
        output_file = split_line[1]
    elif split_line[0] == "ffmpeg_bin":
        ffmpeg_bin = split_line[1]
    elif split_line[0] == "multipass":
        multipass = split_line[1]		
    elif split_line[0] == "stats_file":
        stats_file = split_line[1]

    elif split_line[0] == "mastering_display_sei_present":
        mastering_display_sei_present = split_line[1]
    elif split_line[0] == "mastering_display_sei_x1":
        mastering_display_sei_x1 = split_line[1]
    elif split_line[0] == "mastering_display_sei_x2":
        mastering_display_sei_x2 = split_line[1]
    elif split_line[0] == "mastering_display_sei_x3":
        mastering_display_sei_x3 = split_line[1]
    elif split_line[0] == "mastering_display_sei_y1":
        mastering_display_sei_y1 = split_line[1]
    elif split_line[0] == "mastering_display_sei_y2":
        mastering_display_sei_y2 = split_line[1]
    elif split_line[0] == "mastering_display_sei_y3":
        mastering_display_sei_y3 = split_line[1]
    elif split_line[0] == "mastering_display_sei_wx":
        mastering_display_sei_wx = split_line[1]
    elif split_line[0] == "mastering_display_sei_wy":
        mastering_display_sei_wy = split_line[1]
    elif split_line[0] == "mastering_display_sei_max_lum":
        mastering_display_sei_max_lum = split_line[1]
    elif split_line[0] == "mastering_display_sei_min_lum":
        mastering_display_sei_min_lum = split_line[1]

    elif split_line[0] == "color_description_present":
        color_description_present = split_line[1]
    elif split_line[0] == "color_primaries":
        color_primaries = split_line[1]
    elif split_line[0] == "transfer_characteristics":
        transfer_characteristics = split_line[1]
    elif split_line[0] == "matrix_coefficients":
        matrix_coefficients = split_line[1]

    elif split_line[0] == "light_level_information_sei_present":
        light_level_information_sei_present = split_line[1]
    elif split_line[0] == "light_level_max_content":
        light_level_max_content = split_line[1]
    elif split_line[0] == "light_level_max_frame_average":
        light_level_max_frame_average = split_line[1]
        
command_line = ffmpeg_bin
command_line += " -f rawvideo"
command_line += " -s " + width + "x" + height
command_line += " -pix_fmt " + color_space

if bit_depth == "10":
    command_line += "10le"

command_line += " -framerate " + frame_rate
command_line += " -i " + input_file
command_line += " -c:v libx265 -x265-params \""

if multipass == "1st":
    command_line += "pass=1:"
elif multipass == "last":
    command_line += "pass=2:"
elif multipass == "nth":
    command_line += "pass=3:"

if multipass != "off":
    command_line += "stats=" + stats_file + ":"

if color_description_present == "1":
    command_line += "colorprim=" + color_primaries + ":"
    command_line += "transfer=" + transfer_characteristics + ":"
    command_line += "colormatrix=" + matrix_coefficients + ":"

if light_level_information_sei_present == "1":
    command_line += "max-cll=" + light_level_max_content + "," + light_level_max_frame_average + ":"

if mastering_display_sei_present == "1":
    command_line += "master-display=" 
    command_line += "G(" + mastering_display_sei_x1 + "," + mastering_display_sei_y1 + ")"
    command_line += "B(" + mastering_display_sei_x2 + "," + mastering_display_sei_y2 + ")"
    command_line += "R(" + mastering_display_sei_x3 + "," + mastering_display_sei_y3 + ")"
    command_line += "WP(" + mastering_display_sei_wx + "," + mastering_display_sei_wy + ")"
    command_line += "L(" + mastering_display_sei_max_lum + "," + mastering_display_sei_min_lum + "):"

command_line += "aud=1:annexb=1:repeat-headers=1:hrd=1:hash=1:chromaloc=2:bitrate=" + data_rate + ":vbv-maxrate=" + max_vbv_data_rate + ":vbv-bufsize=" + vbv_buffer_size + "\""
command_line += " -an"
command_line += " -y"
command_line += " -f hevc " + output_file

print(command_line)

