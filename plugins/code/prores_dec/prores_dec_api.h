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

#ifndef __DEE_PLUGINS_PRORES_DEC_API_H__
#define __DEE_PLUGINS_PRORES_DEC_API_H__

#include "common.h"
#include <cstdint>

#define PRORES_DEC_API_VERSION 2

#ifdef __cplusplus
extern "C" {
#endif

/* Framework sets/gets following properties. Each prores_dec plugin should handle them.
*      PROPERTY : TYPE : VALUES : WHERE : COMMENT
* -----------------------------------------------------------------------------
*      width : integer : n/a : prores_dec_init
*      height : integer : n/a : prores_dec_init
*      multithread : integer : n/a : prores_dec_init : number of threads used for decoding, 0 means auto
*      output_format : string :  "yuv422p16le:rgb48le" : prores_dec_init : desired format of decoded data
*      temp_file_num : integer : n/a : prores_dec_get_property : accessed before prores_dec_init, optional, should be used if plugin needs some temp files
*      temp_file : string : n/a : prores_dec_init : occurs multiple times, according to value retrieved from 'temp_file_num'
*
* Additionally, prores_dec_init_params_t structure will contain all plugin-specific properties set via XML interface (ACCESS_TYPE_USER).
*
*/

typedef enum {
    YUV422P16LE,
    RGB48LE,
} ProresDecFormat;

/** @brief Set of properties to init decoder */
typedef struct {
    const Property* properties; /**< Pointer to array of properties */
    int count;                  /**< Number of properties in array */
} ProresDecInitParams;

/** @brief prores_dec input */
typedef struct {
    void* buffer;  /**< Pointer to PRORES frame */
    uint64_t size; /**< Size of PRORES frame */
} ProresDecInput;

/** @brief prores_dec output */
typedef struct {
    void* buffer;           /**< Pointer to decoded frame */
    uint64_t size;          /**< Size of decoded frame */
    int width;              /**< Width of decoded frame */
    int height;             /**< Height of decoded frame */
    ProresDecFormat format; /**< Format of decoded frame */
} ProresDecOutput;

/** @brief Handle to decoder instance
 *  prores_dec_handle_t is actually pointer to decoder context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* ProresDecHandle;

/** @brief Get info about supported properties
 *  @param info Pointer to array with property information
 *  @return Number of properties in info array
 */
typedef int (*ProresDecGetInfo)(const PropertyInfo** info);

/** @brief Create decoder instance
 *  @param handle Decoder instance handle
 *  @return status code
 */
typedef Status (*ProresDecCreate)(ProresDecHandle* handle);

/** @brief Initialize decoder instance
 *  @param handle Decoder instance handle
 *  @param initParams Properties to init decoder instance
 *  @return status code
 */
typedef Status (*ProresDecInit)(ProresDecHandle handle, const ProresDecInitParams* initParams);

/** @brief Close decoder instance
 *  @param handle Decoder instance handle
 *  @return status code
 */
typedef Status (*ProresDecClose)(ProresDecHandle handle);

/** @brief Decode prores picture. Each call must produce decoded picture.
 *  @param handle Decoder instance handle
 *  @param in Encoded input
 *  @param out Decoded output
 *  @return status code
 */
typedef Status (*ProresDecProcess)(ProresDecHandle handle, const ProresDecInput* in, ProresDecOutput* out);

/** @brief Property setter
 *  @param handle Decoder instance handle
 *  @param property Property to write
 *  @return status code
 */
typedef Status (*ProresDecSetProperty)(ProresDecHandle handle, const Property* property);

/** @brief Property getter
 *  @param handle Decoder instance handle
 *  @param property Property to read
 *  @return status code
 */
typedef Status (*ProresDecGetProperty)(ProresDecHandle handle, Property* property);

/** @brief Get string describing last warning or error occurred
 *  @param handle Decoder instance handle
 *  @return Pointer to string with message
 */
typedef const char* (*ProresDecGetMessage)(ProresDecHandle handle);

/** @brief Structure with pointers to all API functions */
typedef struct {
    const char* pluginName;
    ProresDecGetInfo getInfo;
    ProresDecCreate create;
    ProresDecInit init;
    ProresDecClose close;
    ProresDecProcess process;
    ProresDecSetProperty setProperty;
    ProresDecGetProperty getProperty;
    ProresDecGetMessage getMessage;
} ProresDecApi;

/** @brief Export symbol to access implementation of PRORES Decoder plugin
 *  @return pointer to prores_dec_api_t
 */
DLB_EXPORT
ProresDecApi* proresDecGetApi(void);

/** @brief Definition of pointer to proresDecGetApi function */
typedef ProresDecApi* (*ProresDecGetApi)(void);

/** @brief Export symbol to access API version of PRORES Decoder plugin
*  @return integer representing API version
*/
DLB_EXPORT
int proresDecGetApiVersion(void);

/** @brief Definition of pointer to proresDecGetApiVersion function */
typedef int (*ProresDecGetApiVersion)(void);

#ifdef __cplusplus
}
#endif

#endif // __DEE_PLUGINS_PRORES_DEC_API_H__
