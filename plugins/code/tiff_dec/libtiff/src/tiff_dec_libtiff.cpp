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

#include <assert.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "GenericPlugin.h"
#include "SystemCalls.h"
#include "tiff_dec_api.h"
#include "tiffio.h"
#include "tiffio.hxx"

static const int temp_file_num = 0;
static std::string temp_file_num_str = std::to_string(temp_file_num);

static const PropertyInfo tiff_dec_libtiff_info[] = {
    {"plugin_path", PROPERTY_TYPE_STRING, "Path to this plugin.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT},
    {"config_path", PROPERTY_TYPE_STRING, "Path to DEE config file.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT},
    {"temp_file_num", PROPERTY_TYPE_INTEGER, "Indicates how many temp files this plugin requires.",
     temp_file_num_str.c_str(), NULL, 0, 1, ACCESS_TYPE_READ},
    {"temp_file", PROPERTY_TYPE_INTEGER, "Path to temp file.", NULL, NULL, temp_file_num, temp_file_num,
     ACCESS_TYPE_WRITE_INIT},
};

static size_t libtiff_get_info(const PropertyInfo** info) {
    *info = tiff_dec_libtiff_info;
    return sizeof(tiff_dec_libtiff_info) / sizeof(PropertyInfo);
}

typedef struct {
    std::string msg;
    uint16_t* outputBuffer;
    uint16_t* lineBuffer;
    std::vector<std::string> tempFile;
    GenericPlugin genericPlugin;
} tiff_dec_libtiff_data_t;

/* This structure can contain only pointers and simple types */
typedef struct {
    tiff_dec_libtiff_data_t* data;
} tiff_dec_libtiff_t;

static uint64_t libtiff_get_size() {
    return sizeof(tiff_dec_libtiff_t);
}

static void init_data(tiff_dec_libtiff_data_t* data) {
    data->outputBuffer = NULL;
    data->lineBuffer = NULL;
    data->msg.clear();
    data->tempFile.clear();
}

static Status libtiff_init(TiffDecHandle handle, const TiffDecInitParams* init_params) {
    tiff_dec_libtiff_t* state = (tiff_dec_libtiff_t*)handle;
    state->data = new tiff_dec_libtiff_data_t;

    init_data(state->data);

    for (uint64_t i = 0; i < init_params->count; i++) {
        std::string name(init_params->properties[i].name);
        std::string value(init_params->properties[i].value);

        if (state->data->genericPlugin.setProperty(&init_params->properties[i]) == STATUS_OK)
            continue;

        if ("temp_file" == name) {
            state->data->tempFile.push_back(value);
        }
        else {
            state->data->msg += "\nUnknown XML property: " + name;
            return STATUS_ERROR;
        }
    }

    if ((int)state->data->tempFile.size() < temp_file_num) {
        state->data->msg = "Need more temp files.";
        return STATUS_ERROR;
    }

    state->data->msg = "LIBTIFF version: " + std::string(TIFFLIB_VERSION_STR);

    return STATUS_OK;
}

static Status libtiff_close(TiffDecHandle handle) {
    tiff_dec_libtiff_t* state = (tiff_dec_libtiff_t*)handle;

    if (state->data) {
        if (state->data->outputBuffer) {
            delete[] state->data->outputBuffer;
            state->data->outputBuffer = NULL;
        }
        if (state->data->lineBuffer) {
            delete[] state->data->lineBuffer;
            state->data->lineBuffer = NULL;
        }
        delete state->data;
        state->data = NULL;
    }

    return STATUS_OK;
}

struct TiffStream {
    char* data;
    uint64_t size;
    uint64_t pos;
};

tsize_t tiff_Read(thandle_t st, tdata_t buffer, tsize_t size) {
    auto s = (TiffStream*)st;
    auto remainignBytes = s->size - s->pos;
    if (size > (tsize_t)remainignBytes)
        size = (tsize_t)remainignBytes;
    memcpy(buffer, s->data + s->pos, size);
    s->pos += (uint64_t)size;
    return size;
};

tsize_t tiff_Write(thandle_t, tdata_t, tsize_t) {
    return 0;
};

int tiff_Close(thandle_t) {
    return 0;
};

toff_t tiff_Seek(thandle_t st, toff_t pos, int whence) {
    auto s = (TiffStream*)st;
    if (SEEK_SET == whence)
        s->pos = (uint64_t)pos;
    else if (SEEK_CUR == whence)
        s->pos += (uint64_t)pos;

    if (s->pos > s->size)
        s->pos = s->size;

    return (toff_t)s->pos;
};

toff_t tiff_Size(thandle_t st) {
    auto s = (TiffStream*)st;
    return (toff_t)s->size;
};

int tiff_Map(thandle_t, tdata_t*, toff_t*) {
    return 0;
};

void tiff_Unmap(thandle_t, tdata_t, toff_t) {
    return;
};

static Status libtiff_process(TiffDecHandle handle, const TiffDecInput* in, TiffDecOutput* out) {
    tiff_dec_libtiff_t* state = (tiff_dec_libtiff_t*)handle;

    state->data->msg.clear();

    TiffStream stream;
    stream.data = (char*)in->buffer;
    stream.pos = 0;
    stream.size = in->size;

    TIFF* inTiff = TIFFClientOpen("Memory", "r", (thandle_t)&stream, tiff_Read, tiff_Write, tiff_Seek, tiff_Close,
                                  tiff_Size, tiff_Map, tiff_Unmap);

    uint32 width = 0;
    uint32 height = 0;
    uint16 bitsPerSample = 0;
    uint16 nsamples = 0;
    uint16 photometric = 0;
    uint16 subsampling[2] = {0, 0};

    TIFFGetField(inTiff, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(inTiff, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(inTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
    TIFFGetField(inTiff, TIFFTAG_SAMPLESPERPIXEL, &nsamples);
    TIFFGetField(inTiff, TIFFTAG_PHOTOMETRIC, &photometric);

    if (bitsPerSample != 16) {
        state->data->msg = "Unsupported bit-depth: " + std::to_string(bitsPerSample);
        TIFFClose(inTiff);
        return STATUS_ERROR;
    }

    TiffFormat format;
    if (PHOTOMETRIC_RGB == photometric) {
        format = TIFF_FORMAT_RGB48LE;
    }
    else if (PHOTOMETRIC_YCBCR == photometric) {
        TIFFGetField(inTiff, TIFFTAG_YCBCRSUBSAMPLING, &subsampling[0], &subsampling[1]);
        if (1 == subsampling[0] && 1 == subsampling[1]) {
            format = TIFF_FORMAT_YUV444P16LE;
        }
        else {
            state->data->msg =
                "Unsupported subsampling: " + std::to_string(subsampling[0]) + "," + std::to_string(subsampling[1]);
            TIFFClose(inTiff);
            return STATUS_ERROR;
        }
    }
    else {
        state->data->msg = "Unsupported photometric interpretation: " + std::to_string(photometric);
        TIFFClose(inTiff);
        return STATUS_ERROR;
    }
    const int maxPlaneNum = 4;
    uint64_t buffer_size = width * height * maxPlaneNum;
    if (!state->data->outputBuffer)
        state->data->outputBuffer = new uint16_t[buffer_size];
    if (!state->data->lineBuffer)
        state->data->lineBuffer = new uint16_t[buffer_size];

    uint64_t pixNum = width * height;
    memset(state->data->outputBuffer, 0, buffer_size);
    uint16* plane[maxPlaneNum];
    for (int i = 0; i < maxPlaneNum; i++) {
        plane[i] = &state->data->outputBuffer[i * pixNum];
    }

    out->buffer[0] = (void*)plane[0];
    out->buffer[1] = (void*)plane[1];
    out->buffer[2] = (void*)plane[2];

    uint16_t* line = state->data->lineBuffer;
    for (uint32 row = 0; row < height; row++) {
        if (!TIFFReadScanline(inTiff, line, row)) {
            state->data->msg = "TIFFReadScanline failed";
            TIFFClose(inTiff);
            return STATUS_ERROR;
        }

        for (uint32 i = 0; i < width; i++) {
            for (uint32 s = 0; s < nsamples; s++) {
                *(plane[s]++) = line[(i*nsamples)+s];
            }
        }
    }

    out->width = width;
    out->height = height;
    out->format = format;

    TIFFClose(inTiff);
    return STATUS_OK;
}

static Status libtiff_set_property(TiffDecHandle, const Property*) {
    return STATUS_ERROR;
}

static Status libtiff_get_property(TiffDecHandle handle, Property* property) {
    tiff_dec_libtiff_t* state = (tiff_dec_libtiff_t*)handle;
    if (NULL == state)
        return STATUS_ERROR;

    if (NULL != property->name) {
        std::string name(property->name);
        if ("temp_file_num" == name) {
            strcpy(property->value, std::to_string(temp_file_num).c_str());
            return STATUS_OK;
        }
    }
    return STATUS_ERROR;
}

static const char* libtiff_get_message(TiffDecHandle handle) {
    tiff_dec_libtiff_t* state = (tiff_dec_libtiff_t*)handle;
    if (!state->data)
        return NULL;
    return state->data->msg.empty() ? NULL : state->data->msg.c_str();
}

static TiffDecApi libtiff_plugin_api = {
    "libtiff",       libtiff_get_info,     libtiff_get_size,     libtiff_init,       libtiff_close,
    libtiff_process, libtiff_set_property, libtiff_get_property, libtiff_get_message};

DLB_EXPORT
TiffDecApi* tiffDecGetApi() {
    return &libtiff_plugin_api;
}

DLB_EXPORT
int tiffDecGetApiVersion(void) {
    return TIFF_DEC_API_VERSION;
}
