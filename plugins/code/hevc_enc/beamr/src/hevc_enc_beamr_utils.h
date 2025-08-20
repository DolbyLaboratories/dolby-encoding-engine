/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2019, Dolby Laboratories
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

#ifndef __DEE_PLUGINS_HEVC_ENC_BEAMR_UTILS_H__
#define __DEE_PLUGINS_HEVC_ENC_BEAMR_UTILS_H__

#include "hevc_enc_api.h"
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& s, const std::string& delim);
int64_t parseInt(const std::string& name, const std::string& value, const PropertyInfo* schema, size_t count);
std::string parseString(const std::string& name, const std::string& value, const PropertyInfo* schema, size_t count);
std::string parseStringList(const std::string& name,
                            const std::string& value,
                            const std::string& listDelim,
                            const std::string& enums);
bool parseBool(const std::string& name, const std::string& value, const PropertyInfo* schema, size_t count);

int64_t string2int(const std::string& name, const std::string& value, int64_t minValue, int64_t maxValue);
bool string2bool(const std::string& name, const std::string& value);

struct FramePeriod {
    int numUnitsInTick;
    int timeScale;
};

FramePeriod string2FramePeriod(const std::string& s);
int color_primaries2int(const std::string& s);
int transfer_characteristics2int(const std::string& s);
int matrix_coefficients2int(const std::string& s);

void checkFileReadable(const std::string& path);
void checkFileWritable(const std::string& path);

#endif //__DEE_PLUGINS_HEVC_ENC_BEAMR_UTILS_H__
