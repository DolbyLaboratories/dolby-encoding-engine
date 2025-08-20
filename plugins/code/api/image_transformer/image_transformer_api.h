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

#ifndef __DEE_PLUGINS_IMG_TRANSFORMER_API_H__
#define __DEE_PLUGINS_IMG_TRANSFORMER_API_H__

#include "plugins_common.h"
#include <cstdint>
#define IMG_TRANSFORMER_API_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PLANAR = 0,
    INTERLEAVED
}ImgTransformerArrangement;

typedef enum {
    FORMAT_UNKNOWN = 0,
    FORMAT_RAW,
    FORMAT_JPEG2000,
    FORMAT_PRORES,
    FORMAT_TIFF,
}ImgTransformerFormat;

typedef enum {
    CODEC_UNSPECIFIED = 0,
    CODEC_JPEG2000,
    CODEC_PRORES_APCH,
    CODEC_PRORES_APCN,
    CODEC_PRORES_APCS,
    CODEC_PRORES_APCO,
    CODEC_PRORES_AP4H,
    CODEC_PRORES_AP4X,
    CODEC_TIFF,
    CODEC_NONE,
}ImgTransformerCodec;

typedef enum {
    CHROMA_UNKNOWN = 0,
    CHROMA_ACES,   /**< Academy Color Encoding Space primaries */
    CHROMA_DCI,    /**< DCI P3 primaries with 0.314, 0.351 white */
    CHROMA_P3D65,  /**< Standard Pulsar display space */
    CHROMA_REC709, /**< RGB, primaries / white per Rec. 709 */
    CHROMA_REC2020 /**< RGB, primaries / white per Rec. 2020 */
}ImgTransformerChroma;

typedef enum {
    COLOR_SPACE_UNKNOWN = 0,
    COLOR_SPACE_RGB,         /**< RGB */
    COLOR_SPACE_REC709_YUV,  /**< Y'U'V' per Rec. 709 */
    COLOR_SPACE_REC2020_YUV, /**< Y'U'V' per Rec. 2020 */
    COLOR_SPACE_IPTc2        /**< Proposed mobile representation */
}ImgTransformerColorspace;

typedef enum {
    EOTF_UNKNOWN = 0,
    EOTF_PQ,     /**< PQ encoding */
    EOTF_BT1886, /**< Rec. BT1886 gamma */
    EOTF_REC709, /**< Rec. 709 gamma with linear section at bottom */
    EOTF_HLG,
}ImgTransformerEotf;

typedef enum {
    RANGE_UNKNOWN = 0,
    RANGE_COMPUTER, /**< Computer or full range, 0 thru (2^bits - 1)) */
    RANGE_SDI,      /**< SMPTE SDI valid range (16 thru (2^bits - 17)) */
    RANGE_LEGAL /**< SMPTE SDI valid range, scale = 2^(bits - 8), 16*scale thru 235*scale for luma / 240*scale for chroma */
}ImgTransformerRange;

typedef enum {
    BIT_DEPTH_UNKNOWN = 0,
    BIT_DEPTH_UINT8,      /**< 8 bits per color */
    BIT_DEPTH_UINT10_LSB, /**< 10 lower bits of 16 (0 - 1023) */
    BIT_DEPTH_UINT12_LSB, /**< 12 lower bits of 16 (0 - 4095) */
    BIT_DEPTH_UINT14_LSB, /**< 14 lower bits of 16 (0 - 16383) */
    BIT_DEPTH_UINT16      /**< 16 bits per color */
}ImgTransformerBitdepth;

typedef enum {
    SUBSAMPLING_S444 = 0,
    SUBSAMPLING_S422,
    SUBSAMPLING_S420,
}ImgTransformerSubsampling;

typedef struct  {
    int64_t numerator;
    int64_t denominator;
}ImgTransformerPts;

typedef struct  {
    int64_t top;
    int64_t bottom;
    int64_t left;
    int64_t right;
} ImgTransformerLetterbox;

typedef struct {
    double cfr;
    int64_t start;
    int64_t duration;
    int64_t preroll;
    int64_t postroll;
}ImgTransformerProgram;

typedef struct {
    uint8_t* buffer;
    // int64_t bufferSize;
    int64_t size;
} ImgTransformerData;

typedef struct {
    char* metadataFilename;
    int64_t metadataPosition;
    ImgTransformerFormat format;
    ImgTransformerCodec codec;
    ImgTransformerProgram program;
    int64_t width;
    int64_t height;
    ImgTransformerSubsampling subsampling;
    ImgTransformerBitdepth bitdepth;
    ImgTransformerChroma chroma;
    ImgTransformerColorspace colorspace;
    ImgTransformerEotf eotf;
    ImgTransformerRange range;
    ImgTransformerLetterbox letterbox;
    ImgTransformerPts pts;
    ImgTransformerArrangement arrangement;
} ImgTransformerMetadata;

typedef struct {
    ImgTransformerMetadata metadata;
    ImgTransformerData data;
} ImgTransformerFrame;

typedef struct {
    const Property* properties;
    size_t count;
} ImgTransformerInitParams;

typedef void* ImgTransformerHandle;

typedef size_t (*ImgTransformerGetInfo)(const PropertyInfo** info);

typedef size_t (*ImgTransformerGetSize)();

typedef Status (*ImgTransformerInit)(ImgTransformerHandle handle, const ImgTransformerInitParams* initParams);

typedef Status (*ImgTransformerClose)(ImgTransformerHandle handle);

typedef Status (*ImgTransformerProcess) (ImgTransformerHandle handle, ImgTransformerFrame* inFrame);

typedef const char* (*ImgTransformerGetMessage)(ImgTransformerHandle handle);

typedef struct {
    const char* pluginName;
    ImgTransformerGetInfo getInfo;
    ImgTransformerGetSize getSize;
    ImgTransformerInit init;
    ImgTransformerClose close;
    ImgTransformerProcess process;
    ImgTransformerGetMessage getMessage;
} ImgTransformerApi;

DLB_EXPORT
ImgTransformerApi* imgTransformerGetApi();

typedef ImgTransformerApi* (*ImgTransformerGetApi)();

DLB_EXPORT
int imgTransformerGetApiVersion(void);

typedef int (*ImgTransformerGetApiVersion)(void);

#ifdef __cplusplus
}
#endif

#endif // __DLB_PLUGINS_MP4_MUX_API_H__