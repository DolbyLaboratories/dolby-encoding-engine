/*
* BSD 3-Clause License
*
* Copyright (c) 2017-2018, Dolby Laboratories
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

#ifndef __DEE_PLUGINS_HEVC_DEC_API_H__
#define __DEE_PLUGINS_HEVC_DEC_API_H__

#include "common.h"

#define HEVC_DEC_API_VERSION 2

#ifdef __cplusplus
extern "C" {
#endif

/* Framework sets/gets following properties. Each hevc_dec plugin should handle them.
*      PROPERTY : TYPE : VALUES : WHERE : COMMENT
* -----------------------------------------------------------------------------
*   output_format : string  : "any:yuv420_10" : hevc_dec_ffmpeg_init : framework indicates the desired output format of yuv data
*   frame_rate    : integer : "23.976:24:25:29.97:30:48:50:59.94:60" : hevc_dec_ffmpeg_init : framework indicates the framerate of input bitstream
*   temp_file     : string  : n/a : hevc_dec_ffmpeg_init  : framework passes the path to a temporary file created for the plugin, will be called multiple times if the plugin requires multiple temporary files
*   temp_file_num : integer : n/a : hevc_dec_get_property : the framework will try to get this property to know how many temporary files it should allocate for the plugin
*
* Additionally, hevc_dec_init_params_t structure will contain all plugin-specific properties set via XML interface (ACCESS_TYPE_USER).
*
*/

/** @brief  Status code */
typedef enum {
    HEVC_DEC_OK = 0,
    HEVC_DEC_PICTURE_NOT_READY,
    HEVC_DEC_WARNING,
    HEVC_DEC_ERROR,
} HevcDecStatus;

/** @brief Set of properties to init encoder */
typedef struct {
    const Property* properties; /**< Pointer to array of properties */
    size_t count;               /**< Number of properties in array */
} HevcDecInitParams;

/** @brief Frame Rate calculated as frame_period divided by time_scale */
typedef struct {
    int framePeriod;
    int timeScale;
} HevcDecFrameRate;

/** @brief Color Space */
typedef enum {
    HEVC_DEC_COLOR_SPACE_I400 = 0,
    HEVC_DEC_COLOR_SPACE_I420,
    HEVC_DEC_COLOR_SPACE_I422,
    HEVC_DEC_COLOR_SPACE_I444,
} HevcDecColorSpace;

/** @brief Type of video frame */
typedef enum {
    HEVC_DEC_FRAME_TYPE_AUTO = 0,
    HEVC_DEC_FRAME_TYPE_IDR,
    HEVC_DEC_FRAME_TYPE_I,
    HEVC_DEC_FRAME_TYPE_P,
    HEVC_DEC_FRAME_TYPE_B,
    HEVC_DEC_FRAME_TYPE_BREF,
} HevcDecFrameType;

/** @brief Picture structure
 *  Points to video data and describes how it should be interpreted.
 */
typedef struct {
    int bitDepth;      /**< 8 or 10 bits */
    size_t lumaSize;   /**< Y plane data size in bytes */
    size_t chromaSize; /**< Cb and Cr plane data size in bytes */
    void* plane[3];    /**< Pointers to Y, Cb and Cr planes */
    int stride[3];     /**< Number of bytes between rows (for each plane) */

    size_t width;
    size_t height;
    HevcDecColorSpace colorSpace;
    HevcDecFrameType frameType;
    HevcDecFrameRate frameRate;
    int transferCharacteristics;
    int matrixCoeffs;
} HevcDecPicture;

/** @brief Handle to decoder instance
 *  hevc_dec_handle_t is actually pointer to decoder context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* HevcDecHandle;

/** @brief Get info about supported properties
 *  @return Number of properties in info array
 */
typedef size_t (*HevcDecGetInfo)(const PropertyInfo** info); /**< [out] Pointer to array with property information */

/** @brief Get size of decoder context
 *  @return Size in bytes
 */
typedef size_t (*HevcDecGetSize)();

/** @brief Initialize decoder instance
 *  @return status code
 */
typedef HevcDecStatus (*HevcDecInit)(HevcDecHandle handle,                 /**< [in/out] Decoder instance handle */
                                     const HevcDecInitParams* initParams); /**< [in] Properties to init decoder instance */

/** @brief Close decoder instance
 *  @return status code
 */
typedef HevcDecStatus (*HevcDecClose)(HevcDecHandle handle); /**< [in/out] Decoder instance handle */

/** @brief pass a chunk of bitsteam data to decoder and retrieve one decoded picture if available
 *  @return status code
 */
typedef HevcDecStatus (*HevcDecProcess)(HevcDecHandle handle,     /**< [in/out] Encoder instance handle */
                                        const void* streamBuffer, /**< [in] Pointer to buffer containing hevc stream */
                                        const size_t bufferSize,  /**< [in] Size of hevc buffer */
                                        HevcDecPicture* output,   /**< [out] Output buffer */
                                        int* bufferConsumed);     /**< [out] 1 means the input was read, 0 means it has to be sent again */

/** @brief Flush decoder
 *  @return status code
 */
typedef HevcDecStatus (*HevcDecFlush)(HevcDecHandle handle,   /**< [in/out] Decoder instance handle */
                                      HevcDecPicture* output, /**< [in/out] Output buffer */
                                      int* isEmpty);          /**< [out] Flush indicator */

/** @brief Property setter
 *  @return status code
 */
typedef HevcDecStatus (*HevcDecSetProperty)(HevcDecHandle handle,  /**< [in/out] Decoder instance handle */
                                            const Property* prop); /**< [in] Property to write */

/** @brief Property getter
 *  @return status code
 */
typedef HevcDecStatus (*HevcDecGetProperty)(HevcDecHandle handle, /**< [in/out] Decoder instance handle */
                                            Property* prop);      /**< [in/out] Property to read */

/** @brief Get string describing last warning or error occurred
 *  @return Pointer to string with message
 */
typedef const char* (*HevcDecGetMessage)(HevcDecHandle handle); /**< [in/out] Decoder instance handle */

/** @brief Structure with pointers to all API functions */
typedef struct {
    const char* pluginName;
    HevcDecGetInfo getInfo;
    HevcDecGetSize getSize;
    HevcDecInit init;
    HevcDecClose close;
    HevcDecProcess process;
    HevcDecFlush flush;
    HevcDecSetProperty setProperty;
    HevcDecGetProperty getProperty;
    HevcDecGetMessage getMessage;
} HevcDecApi;

/** @brief Export symbol to access implementation of HEVC Decoder plugin
 *  @return pointer to hevc_dec_api_t
 */
DLB_EXPORT
HevcDecApi* hevcDecGetApi();

/** @brief Definition of pointer to hevcDecGetApi function */
typedef HevcDecApi* (*HevcDecGetApi)();

/** @brief Export symbol to access API version of HEVC Decoder plugin
 *  @return integer representing API version
 */
DLB_EXPORT
int hevcDecGetApiVersion(void);

/** @brief Definition of pointer to hevcDecGetApiVersion function */
typedef int (*HevcDecGetApiVersion)(void);

#ifdef __cplusplus
}
#endif

#endif // __DEE_PLUGINS_HEVC_DEC_API_H__
