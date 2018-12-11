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

#ifndef __DLB_PLUGINS_SCALING_API_H__
#define __DLB_PLUGINS_SCALING_API_H__

#include "common.h"

#define SCALING_API_VERSION 2

#ifdef __cplusplus
extern "C" {
#endif

/* Framework sets/gets following properties. Each scaling plugin should handle them.
*      PROPERTY : TYPE : VALUES : WHERE : COMMENT
* -----------------------------------------------------------------------------
*      format : enum : [rgb_16,yuv420_10] : scaling_init : rgb_16 support is obligatory, yuv420_10 is required only if plugin will be used with multi-resolution encoding feature
*      source_width : integer : n/a : scaling_init
*      source_height : integer : n/a : scaling_init
*      target_width : integer : n/a : scaling_init
*      target_height : integer : n/a : scaling_init
*      temp_file_num : integer : n/a : scaling_get_property : accessed before scaling_init, optional, should be used if plugin needs some temp files
*      temp_file : string : n/a : scaling_init : occurs multiple times, according to value retrieved from 'temp_file_num'
*      source_offset_top : integer : n/a : scaling_init : the active area padding for the top of the picture
*      source_offset_bottom : integer : n/a : scaling_init : the active area padding for the bottom of the picture
*      source_offset_left : integer : n/a : scaling_init : the active area padding for the left of the picture
*      source_offset_right : integer : n/a : scaling_init : the active area padding for the right of the picture
*      allow_crop : bool : n/a : scaling_init : when set to false the plugin should not do any cropping or padding
*      get_target_offset_top : integer : n/a : scaling_get_property : accessed to get the active area top offset size after scaling
*      get_target_offset_bottom : integer : n/a : scaling_get_property : accessed to get the active area bottom offset size after scaling
*      get_target_offset_left : integer : n/a : scaling_get_property : accessed to get the active area left offset size after scaling
*      get_target_offset_right : integer : n/a : scaling_get_property : accessed to get the active area right offset size after scaling
*
* Additionally, scaling_init_params_t structure will contain all plugin-specific properties set via XML interface (ACCESS_TYPE_USER).
*
*/

/** @brief Set of properties to init filter */
typedef struct {
    const Property* properties; /**< Pointer to array of properties */
    size_t count;               /**< Number of properties in array */
} ScalingInitParams;

/** @brief Format of input/output picture */
typedef enum {
    SCALING_RGB_16 = 0, /**< Picture stored as independent RGB planes, bit-depth 16 */
    SCALING_YUV420_10   /**< Picture stored as independent YUV planes, bit-depth 10 */
} ScalingPicFormat;

/** @brief scaling input/output picture */
typedef struct {
    size_t width;    /**< Width of picture */
    size_t height;   /**< Height of picture */
    void* buffer[3]; /**< Pointers to RGB or YUV planes */
} ScalingPic;

/** @brief Handle to filter instance
 *  scaling_handle_t is actually pointer to filter context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* ScalingHandle;

/** @brief Get info about supported properties
 *  @return Number of properties in info array
 */
typedef size_t (*ScalingGetInfo)(const PropertyInfo** info); /**< [out] Pointer to array with property information */

/** @brief Get size of filter context
 *  @return Size in bytes
 */
typedef size_t (*ScalingGetSize)(void);

/** @brief Initialize filter instance
 *  @return status code
 */
typedef Status (*ScalingInit)(ScalingHandle handle,                 /**< [in/out] filter instance handle */
                                 const ScalingInitParams* initParams); /**< [in] Properties to init filter instance */

/** @brief Close filter instance
 *  @return status code
 */
typedef Status (*ScalingClose)(ScalingHandle handle); /**< [in/out] filter instance handle */

/** @brief Process picture
 *  @return status code
 */
typedef Status (*ScalingProcess)(ScalingHandle handle, /**< [in/out] filter instance handle */
                                    const ScalingPic* in, /**< [in] Input picture */
                                    ScalingPic* out);     /**< [out] Output picture, format reflects input picture */

/** @brief Property setter
 *  @return status code
 */
typedef Status (*ScalingSetProperty)(ScalingHandle handle,   /**< [in/out] filter instance handle */
                                        const Property* property); /**< [in] Property to write */

/** @brief Property getter
 *  @return status code
 */
typedef Status (*ScalingGetProperty)(ScalingHandle handle, /**< [in/out] filter instance handle */
                                        Property* property);     /**< [in/out] Property to read */

/** @brief Get string describing last warning or error occurred
 *  @return Pointer to string with message
 */
typedef const char* (*ScalingGetMessage)(ScalingHandle handle); /**< [in/out] filter instance handle */

/** @brief Structure with pointers to all API functions */
typedef struct {
    const char* pluginName;
    ScalingGetInfo getInfo;
    ScalingGetSize getSize;
    ScalingInit init;
    ScalingClose close;
    ScalingProcess process;
    ScalingSetProperty setProperty;
    ScalingGetProperty getProperty;
    ScalingGetMessage getMessage;
} ScalingApi;

/** @brief Export symbol to access implementation of Scaling Filter plugin
 *  @return pointer to scaling_api_t
 */
DLB_EXPORT
ScalingApi* scalingGetApi(void);

/** @brief Definition of pointer to scalingGetApi function */
typedef ScalingApi* (*ScalingGetApi)(void);

/** @brief Export symbol to access API version of scaling Filter plugin
 *  @return integer representing API version
 */
DLB_EXPORT
int scalingGetApiVersion(void);

/** @brief Definition of pointer to scalingGetApiVersion function */
typedef int (*ScalingGetApiVersion)(void);

#ifdef __cplusplus
}
#endif

#endif // __DLB_PLUGINS_SCALING_API_H__
