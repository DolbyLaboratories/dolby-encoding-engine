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

#ifndef __DLB_PLUGINS_NOISE_API_H__
#define __DLB_PLUGINS_NOISE_API_H__

#include "common.h"

#define NOISE_API_VERSION 2

#ifdef __cplusplus
extern "C" {
#endif

/* Framework sets/gets following properties. Each noise plugin should handle them.
*      PROPERTY : TYPE : VALUES : WHERE : COMMENT
* -----------------------------------------------------------------------------
*      source_width : integer : n/a : noise_init
*      source_height : integer : n/a : noise_init
*      temp_file_num : integer : n/a : noise_get_property : accessed before noise_init, optional, should be used if plugin needs some temp files
*      temp_file : string : n/a : noise_init : occurs multiple times, according to value retrieved from 'temp_file_num'
*
* Additionally, noise_init_params_t structure will contain all plugin-specific properties set via XML interface (ACCESS_TYPE_USER).
*
*/

/** @brief Set of properties to init filter */
typedef struct {
    const Property* properties; /**< Pointer to array of properties */
    size_t count;               /**< Number of properties in array */
} NoiseInitParams;

/** @brief noise input/output picture */
typedef struct {
    size_t width;    /**< Width of picture */
    size_t height;   /**< Height of picture */
    void* buffer[3]; /**< Pointers to RGB planes, 16 bits per pixel */
} NoisePic;

/** @brief Handle to filter instance
 *  noise_handle_t is actually pointer to filter context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* NoiseHandle;

/** @brief Get info about supported properties
 *  @return Number of properties in info array
 */
typedef size_t (*NoiseGetInfo)(const PropertyInfo** info); /**< [out] Pointer to array with property information */

/** @brief Get size of filter context
 *  @return Size in bytes
 */
typedef size_t (*NoiseGetSize)(void);

/** @brief Initialize filter instance
 *  @return status code
 */
typedef Status (*NoiseInit)(NoiseHandle handle,                 /**< [in/out] filter instance handle */
                               const NoiseInitParams* initParams); /**< [in] Properties to init filter instance */

/** @brief Close filter instance
 *  @return status code
 */
typedef Status (*NoiseClose)(NoiseHandle handle); /**< [in/out] filter instance handle */

/** @brief Process picture
 *  @return status code
 */
typedef Status (*NoiseProcess)(NoiseHandle handle, /**< [in/out] filter instance handle */
                                  const NoisePic* in, /**< [in] Input picture */
                                  NoisePic* out);     /**< [out] Output picture */

/** @brief Property setter
 *  @return status code
 */
typedef Status (*NoiseSetProperty)(NoiseHandle handle,       /**< [in/out] filter instance handle */
                                      const Property* properties); /**< [in] Property to write */

/** @brief Property getter
 *  @return status code
 */
typedef Status (*NoiseGetProperty)(NoiseHandle handle, /**< [in/out] filter instance handle */
                                      Property* properties /**< [in/out] Property to read */);

/** @brief Get string describing last warning or error occurred
 *  @return Pointer to string with message
 */
typedef const char* (*NoiseGetMessage)(NoiseHandle handle); /**< [in/out] filter instance handle */

/** @brief Structure with pointers to all API functions */
typedef struct {
    const char* pluginName;
    NoiseGetInfo getInfo;
    NoiseGetSize getSize;
    NoiseInit init;
    NoiseClose close;
    NoiseProcess process;
    NoiseSetProperty setProperty;
    NoiseGetProperty getProperty;
    NoiseGetMessage getMessage;
} NoiseApi;

/** @brief Export symbol to access implementation of Noise Filter plugin
 *  @return pointer to noise_api_t
 */
DLB_EXPORT
NoiseApi* noiseGetApi(void);

/** @brief Definition of pointer to noiseGetApi function */
typedef NoiseApi* (*NoiseGetApi)(void);

/** @brief Export symbol to access API version of Noise Filter plugin
 *  @return integer representing API version
 */
DLB_EXPORT
int noiseGetApiVersion(void);

/** @brief Definition of pointer to noiseGetApiVersion function */
typedef int (*NoiseGetApiVersion)(void);

#ifdef __cplusplus
}
#endif

#endif // __DLB_PLUGINS_NOISE_API_H__
