#!/usr/bin/env python
import sys

file_name = sys.argv[1]

with open(file_name) as file:
    content = file.readlines()

content = [x.strip() for x in content]

bit_depth = "8"
width = "0"
height = "0"
ffmpeg_bin = "ffmpeg.exe" 
input_file = "input_file"
output_file = "output_file"

for x in content:
    x.strip()
    split_line = x.split("=", 1)
    
    if split_line[0] == "output_bitdepth":
        bit_depth = split_line[1]
    elif split_line[0] == "width":
        width = split_line[1]
    elif split_line[0] == "height":
        height = split_line[1]
    elif split_line[0] == "input_file":
        input_file = split_line[1]
    elif split_line[0] == "output_file":
        output_file = split_line[1]
    elif split_line[0] == "ffmpeg_bin":
        ffmpeg_bin = split_line[1]

command_line = ffmpeg_bin

command_line += " -y -f hevc"
command_line += " -i " + input_file

command_line += " -f rawvideo"
if bit_depth == "8":
    command_line += " -pix_fmt yuv420p"
else:
    command_line += " -pix_fmt yuv420p10le"

command_line += " -vf scale=" + width + ":" + height + ":force_original_aspect_ratio=decrease,pad=" + width + ":" + height + ":\(ow-iw\)/2:\(oh-ih\)/2 "
command_line += output_file

print(command_line)
