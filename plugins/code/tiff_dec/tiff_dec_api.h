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

#ifndef __DEE_PLUGINS_TIFF_DEC_API_H__
#define __DEE_PLUGINS_TIFF_DEC_API_H__

#include "common.h"

#define TIFF_DEC_API_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

/* Framework sets/gets following properties. Each tiff_dec plugin should handle them.
 *      PROPERTY : TYPE : VALUES : WHERE : COMMENT
 * -----------------------------------------------------------------------------
 *      temp_file_num : integer : n/a : tiff_dec_get_property : accessed before tiff_dec_init, optional, should be used
 * if plugin needs some temp files temp_file : string : n/a : tiff_dec_init : occurs multiple times, according to value
 * retrieved from 'temp_file_num'
 *
 * Additionally, tiff_dec_init_params_t structure will contain all plugin-specific properties set via XML interface
 * (ACCESS_TYPE_USER).
 *
 */

/** @brief Set of properties to init decoder */
typedef struct {
    const Property* properties; /**< Pointer to array of properties */
    uint64_t count;             /**< Number of properties in array */
} TiffDecInitParams;

/** @brief tiff_dec input */
typedef struct {
    void* buffer;  /**< Pointer to TIFF frame */
    uint64_t size; /**< Size of TIFF frame */
} TiffDecInput;

/** @brief Format of output picture. Formats follow ffmpeg convention. */
typedef enum {
    TIFF_FORMAT_RGB48LE = 0, /**< Planar RGB, 16 bits per component. */
    TIFF_FORMAT_YUV444P16LE, /**< Planar YUV 444, 16 bits per component. */
    TIFF_FORMAT_YUV422P16LE, /**< Planar YUV 422, 16 bits per component. */
    TIFF_FORMAT_YUV420P16LE  /**< Planar YUV 420, 16 bits per component. */
} TiffFormat;

/** @brief tiff_dec output */
typedef struct {
    uint64_t width;       /**< Width of decoded picture */
    uint64_t height;      /**< Height of decoded picture */
    void* buffer[3];      /**< Pointers to RGB or YUV components. */
    TiffFormat format;    /**< Format of output picture. */
} TiffDecOutput;

/** @brief Handle to decoder instance
 *  tiff_dec_handle_t is actually pointer to decoder context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* TiffDecHandle;

/** @brief Get info about supported properties
 *  @return Number of properties in info array
 */
typedef size_t (*TiffDecGetInfo)(const PropertyInfo** info); /**< [out] Pointer to array with property information */

/** @brief Get size of decoder context
 *  @return Size in bytes
 */
typedef uint64_t (*TiffDecGetSize)(void);

/** @brief Initialize decoder instance
 *  @return status code
 */
typedef Status (*TiffDecInit)(TiffDecHandle handle,                 /**< [in/out] Decoder instance handle */
                              const TiffDecInitParams* initParams); /**< [in] Properties to init decoder instance */

/** @brief Close decoder instance
 *  @return status code
 */
typedef Status (*TiffDecClose)(TiffDecHandle handle); /**< [in/out] Decoder instance handle */

/** @brief Decode tiff picture. Each call must produce decoded picture.
 *  @return status code
 */
typedef Status (*TiffDecProcess)(TiffDecHandle handle,   /**< [in/out] Decoder instance handle */
                                 const TiffDecInput* in, /**< [in] Encoded input */
                                 TiffDecOutput* out);    /**< [out] Decoded output */

/** @brief Property setter
 *  @return status code
 */
typedef Status (*TiffDecSetProperty)(TiffDecHandle handle,      /**< [in/out] Decoder instance handle */
                                     const Property* property); /**< [in] Property to write */

/** @brief Property getter
 *  @return status code
 */
typedef Status (*TiffDecGetProperty)(TiffDecHandle handle, /**< [in/out] Decoder instance handle */
                                     Property* property);  /**< [in/out] Property to read */

/** @brief Get string describing last warning or error occurred
 *  @return Pointer to string with message
 */
typedef const char* (*TiffDecGetMessage)(TiffDecHandle handle); /**< [in/out] Decoder instance handle */

/** @brief Structure with pointers to all API functions */
typedef struct {
    const char* pluginName;
    TiffDecGetInfo getInfo;
    TiffDecGetSize getSize;
    TiffDecInit init;
    TiffDecClose close;
    TiffDecProcess process;
    TiffDecSetProperty setProperty;
    TiffDecGetProperty getProperty;
    TiffDecGetMessage getMessage;
} TiffDecApi;

/** @brief Export symbol to access implementation of TIFF Decoder plugin
 *  @return pointer to tiff_dec_api_t
 */
DLB_EXPORT
TiffDecApi* tiffDecGetApi(void);

/** @brief Definition of pointer to tiffDecGetApi function */
typedef TiffDecApi* (*TiffDecGetApi)(void);

/** @brief Export symbol to access API version of tiff decoder plugin
 *  @return integer representing API version
 */
DLB_EXPORT
int tiffDecGetApiVersion(void);

/** @brief Definition of pointer to tiffDecGetApiVersion function */
typedef int (*TiffDecGetApiVersion)(void);

#ifdef __cplusplus
}
#endif

#endif // __DEE_PLUGINS_TIFF_DEC_API_H__
