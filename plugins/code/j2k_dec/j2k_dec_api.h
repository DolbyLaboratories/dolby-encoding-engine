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

#ifndef __DEE_PLUGINS_J2K_DEC_API_H__
#define __DEE_PLUGINS_J2K_DEC_API_H__

#include "common.h"

#define J2K_DEC_API_VERSION 2

#ifdef __cplusplus
extern "C" {
#endif

/* Framework sets/gets following properties. Each j2k_dec plugin should handle them.
*      PROPERTY : TYPE : VALUES : WHERE : COMMENT
* -----------------------------------------------------------------------------
*      width : integer : n/a : j2k_dec_init
*      height : integer : n/a : j2k_dec_init
*      temp_file_num : integer : n/a : j2k_dec_get_property : accessed before j2k_dec_init, optional, should be used if plugin needs some temp files
*      temp_file : string : n/a : j2k_dec_init : occurs multiple times, according to value retrieved from 'temp_file_num'
*
* Additionally, j2k_dec_init_params_t structure will contain all plugin-specific properties set via user's configuration (ACCESS_TYPE_USER).
*
*/

/** @brief Set of properties to init decoder */
typedef struct {
    const Property* properties; /**< Pointer to array of properties */
    size_t count;               /**< Number of properties in array */
} J2kDecInitParams;

/** @brief j2k_dec input */
typedef struct {
    void* buffer; /**< Pointer to J2K frame */
    size_t size;  /**< Size of J2K frame */
} J2kDecInput;

/** @brief j2k_dec output */
typedef struct {
    size_t width;    /**< Width of decoded picture */
    size_t height;   /**< Height of decoded picture */
    void* buffer[3]; /**< Pointers to RGB planes, 16 bits per pixel */
} J2kDecOutput;

/** @brief Handle to decoder instance
 *  j2k_dec_handle_t is actually pointer to decoder context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* J2kDecHandle;

/** @brief Get info about supported properties
 *  @return Number of properties in info array
 */
typedef size_t (*J2kDecGetInfo)(const PropertyInfo** info); /**< [out] Pointer to array with property information */

/** @brief Get size of decoder context
 *  @return Size in bytes
 */
typedef size_t (*J2kDecGetSize)(void);

/** @brief Initialize decoder instance
 *  @return status code
 */
typedef Status (*J2kDecInit)(J2kDecHandle handle,                 /**< [in/out] Decoder instance handle */
                             const J2kDecInitParams* initParams); /**< [in] Properties to init decoder instance */

/** @brief Close decoder instance
 *  @return status code
 */
typedef Status (*J2kDecClose)(J2kDecHandle handle); /**< [in/out] Decoder instance handle */

/** @brief Decode j2k picture. Each call must produce decoded picture.
 *  @return status code
 */
typedef Status (*J2kDecProcess)(J2kDecHandle handle,   /**< [in/out] Decoder instance handle */
                                const J2kDecInput* in, /**< [in] Encoded input */
                                J2kDecOutput* out);    /**< [out] Decoded output */

/** @brief Property setter
 *  @return status code
 */
typedef Status (*J2kDecSetProperty)(J2kDecHandle handle,       /**< [in/out] Decoder instance handle */
                                    const Property* property); /**< [in] Property to write */

/** @brief Property getter
 *  @return status code
 */
typedef Status (*J2kDecGetProperty)(J2kDecHandle handle, /**< [in/out] Decoder instance handle */
                                    Property* property); /**< [in/out] Property to read */

/** @brief Get string describing last warning or error occurred
 *  @return Pointer to string with message
 */
typedef const char* (*J2kDecGetMessage)(J2kDecHandle handle); /**< [in/out] Decoder instance handle */

/** @brief Structure with pointers to all API functions */
typedef struct {
    const char* pluginName;
    J2kDecGetInfo getInfo;
    J2kDecGetSize getSize;
    J2kDecInit init;
    J2kDecClose close;
    J2kDecProcess process;
    J2kDecSetProperty setProperty;
    J2kDecGetProperty getProperty;
    J2kDecGetMessage getMessage;
} J2kDecApi;

/** @brief Export symbol to access implementation of J2K Decoder plugin
 *  @return pointer to j2k_dec_api_t
 */
DLB_EXPORT
J2kDecApi* j2kDecGetApi(void);

/** @brief Definition of pointer to j2kDecGetApi function */
typedef J2kDecApi* (*J2kDecGetApi)(void);

/** @brief Export symbol to access API version of j2k decoder plugin
 *  @return integer representing API version
 */
DLB_EXPORT
int j2kDecGetApiVersion(void);

/** @brief Definition of pointer to j2kDecGetApiVersion function */
typedef int (*J2kDecGetApiVersion)(void);

#ifdef __cplusplus
}
#endif

#endif // __DEE_PLUGINS_J2K_DEC_API_H__
