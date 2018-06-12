#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
/*
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
*/
"""
__author__ = 'Marek Gaik'

import sys
import os
import argparse
import subprocess
import platform
from shutil import copyfile
from shutil import make_archive
import shutil
from distutils.dir_util import copy_tree
import zipfile
import stat
import datetime

windows_target_64   = 'windows_amd64_msvs'
linux_target_64     = 'linux_amd64_gnu'

msbuild = r"C:\Program Files (x86)\MSBuild\12.0\Bin\MSBuild.exe"
make = 'make'

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

"""
Please comment out (#) plugins you wish to exclude from compilation
"""
PLUGINS_DICT = {
'hevc_dec_ffmpeg' : 'hevc_dec/ffmpeg/make/hevc_dec_ffmpeg',
'hevc_enc_x265' : 'hevc_enc/x265/make/hevc_enc_x265/',
'hevc_enc_ffmpeg' : 'hevc_enc/ffmpeg/make/hevc_enc_ffmpeg',
'j2k_dec_ffmpeg' : 'j2k_dec/ffmpeg/make/j2k_dec_ffmpeg',
'j2k_dec_kakadu' : 'j2k_dec/kakadu/make/j2k_dec_kakadu',
'noise_example' : 'noise/noise_example/make/noise_example',
'scaling_example' : 'scaling/scaling_example/make/scaling_example',
#disabled_plugin_example : 'disabled',
}

example ='''
examples:
Linux:   python build_plugins.py --x265 ~/_X265ROOT/ --kakadu ~/_KDUROOT/ --dir dolby-encoding-engine/plugins/code/
Windows: python build_plugins.py --x265 C:\Users\usr\_X265ROOT\ --kakadu C:\Users\mgaik\_KDUROOT\ --dir C:\Users\usr\dolby-encoding-engine\plugins\code
'''

cmdline_header = '''
Dolby Encoding Engine Plugins Build Script.
Please edit PLUGINS_DICT if you wish to disable some plugins.
'''

def main_build_plugins():
    ENV = os.environ
    os_name = platform.system()

    need_x265_root = 'hevc_enc_x265' in PLUGINS_DICT.keys() and 'X265ROOT' not in ENV.keys()
    need_kdu_root = 'j2k_dec_kakadu' in PLUGINS_DICT.keys() and 'KDUROOT' not in ENV.keys()

    parser = argparse.ArgumentParser(description=cmdline_header,
                                     epilog=example,
                                     formatter_class=argparse.RawDescriptionHelpFormatter
                                    )
    parser.add_argument('--dir', help='Plugins base (code) directory. default={}'.format(SCRIPT_DIR),
                        required=False, default=SCRIPT_DIR)
    parser.add_argument('--config', help='Specify build configuration default=release', required=False,
                        choices=['debug', 'release'], default='release')
    parser.add_argument('--no-rebuild', help='Dont clean targets before rebuilding', action='store_true')
    parser.add_argument('--dry', help='Don\'t build, display only build commands', action='store_true')
    parser.add_argument('--x265', help='Specify directory with x265 prerequisites (Optional if X265ROOT is set)', required=need_x265_root)
    parser.add_argument('--kakadu', help='Specify directory with Kakadu prerequisites (Optional if KDUROOT is set)', required=need_kdu_root)

    args = parser.parse_args()
    rebuild = not args.no_rebuild
    cmd_print_only = args.dry

    if need_x265_root or args.x265 is not None:
        ENV['X265ROOT'] = args.x265
        print('X265ROOT={}'.format(ENV['X265ROOT']))
    if need_kdu_root or args.kakadu is not None:
        ENV['KDUROOT'] = args.kakadu
        print('KDUROOT={}'.format(ENV['KDUROOT']))

    results = []
    for plugin in PLUGINS_DICT:
        def get_build_cmd(os_name):
            build_cmd = []
            if os_name == 'Windows':
                solution_file = '{}_2013.sln'.format(plugin)
                full_solution_path = os.path.join(args.dir,
                                                  PLUGINS_DICT[plugin],
                                                  windows_target_64,
                                                  solution_file)
                build_cmd = [msbuild,
                             os.path.abspath(full_solution_path),
                             '/p:Configuration='+args.config,
                            ]
                if rebuild:
                    build_cmd.append('/t:Clean;Build')
            elif os_name == 'Linux':
                makefile_dir = os.path.join(args.dir,
                                            PLUGINS_DICT[plugin],
                                            linux_target_64)
                make_dir_switch = '--directory={}'.format(makefile_dir)
                make_target = plugin + '_' + args.config + '.so'
                build_cmd = [make,
                             make_dir_switch
                            ]
                if rebuild:
                    build_cmd.append('clean')
                build_cmd.append(make_target)
            return build_cmd

        build_cmd = get_build_cmd(os_name)
        return_code = 0
        if not cmd_print_only:
            print('Executing build cmd: {}'.format(' '.join(build_cmd)))
            return_code = subprocess.call(build_cmd, env=ENV)

        results.append('plugin: {}'.format(plugin))
        results.append('build cmd: {}'.format(' '.join(build_cmd)))
        results.append('returned: {}'.format(return_code))

    print('Results for {}: '.format(os_name))
    print("\n\r".join(results))

if __name__=="__main__":
    sys.exit(main_build_plugins())

