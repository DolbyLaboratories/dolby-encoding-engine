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

#ifndef __DEE_PLUGINS_HEVC_ENC_API_H__
#define __DEE_PLUGINS_HEVC_ENC_API_H__

#include "common.h"

#define HEVC_ENC_API_VERSION 2

#ifdef __cplusplus
extern "C" {
#endif

/* Framework sets/gets following properties. Each hevc_enc plugin should handle them. 
*      PROPERTY : TYPE : VALUES : WHERE : COMMENT
* -----------------------------------------------------------------------------
*      width : integer : n/a : hevc_enc_init
*      height : integer : n/a : hevc_enc_init
*      bit_depth : enum : [8,10] : hevc_enc_init : in DEE it is always '10'
*      frame_rate : enum : [23.976,24,25,29.97,30,48,59.94,60] : hevc_enc_init 
*      color_space : enum : [i400,i420,i422,i444] : hevc_enc_init : in DEE it is always 'i420'
*      data_rate : integer : n/a : hevc_enc_init : value in kbps
*      max_vbv_data_rate : integer : n/a : hevc_enc_init : value in kbps
*      vbv_buffer_size : integer : n/a : hevc_enc_init : value in kb
*      range : enum : [limited,full] : hevc_enc_init
*      multi_pass : enum : [off,1st,nth,last] : hevc_enc_init
*      stats_file : string : n/a : hevc_enc_init : temp file to write stats in multipass encoding
*      max_output_data : integer : n/a : hevc_enc_set_property : framework indicates max size of buffer before each 'process' call
*      max_pass_num : integer : n/a : hevc_enc_get_property : accessed before hevc_enc_init
*      absolute_pass_num : integer : n/a : hevc_enc_init : when encoding multiple times (in multipass encoding or in multiple output streams) this value states the index number of the current encoding
*      temp_file_num : integer : n/a : hevc_enc_get_property : accessed before hevc_enc_init, optional, should be used if plugin needs some temp files
*      temp_file : string : n/a : hevc_enc_init : occurs multiple times, according to value retrieved from 'temp_file_num'
*      color_primaries : enum : [unspecified,bt_709,bt_601_625,bt_601_525,bt_2020] : hevc_enc_init : optional, used when encoding HDR10-compatible streams
*      transfer_characteristics : enum : [unspecified,bt_709,bt_601_625,bt_601_525,smpte_st_2084,std_b67] : hevc_enc_init : optional, used when encoding HDR10-compatible streams
*      matrix_coefficients : enum : [unspecified,bt_709,bt_601_625,bt_601_525,bt_2020] : hevc_enc_init : optional, used when encoding HDR10-compatible streams
*      mastering_display_sei_x1 : integer : [0,50000] : hevc_enc_init : first primary x : optional, used when encoding HDR10-compatible streams
*      mastering_display_sei_y1 : integer : [0,50000] : hevc_enc_init : first primary y : optional, used when encoding HDR10-compatible streams
*      mastering_display_sei_x2 : integer : [0,50000] : hevc_enc_init : second primary x : optional, used when encoding HDR10-compatible streams
*      mastering_display_sei_y2 : integer : [0,50000] : hevc_enc_init : second primary y : optional, used when encoding HDR10-compatible streams
*      mastering_display_sei_x3 : integer : [0,50000] : hevc_enc_init : third primary x : optional, used when encoding HDR10-compatible streams
*      mastering_display_sei_y3 : integer : [0,50000] : hevc_enc_init : third primary y : optional, used when encoding HDR10-compatible streams
*      mastering_display_sei_wx : integer : [0,50000] : hevc_enc_init : white point x : optional, used when encoding HDR10-compatible streams
*      mastering_display_sei_wy : integer : [0,50000] : hevc_enc_init : white point y : optional, used when encoding HDR10-compatible streams
*      mastering_display_sei_max_lum : integer : [0:2000000000] : hevc_enc_init : maximum display luminance : optional, used when encoding HDR10-compatible streams
*      mastering_display_sei_min_lum : integer : [0:2000000000] : hevc_enc_init : minimum display luminance : optional, used when encoding HDR10-compatible streams
*      light_level_max_content : integer : [0,65535] : hevc_enc_init : optional, used when encoding HDR10-compatible streams
*      light_level_max_frame_average : integer : [0,65535] : hevc_enc_init : optional, used when encoding HDR10-compatible streams
*      force_slice_type : [true,false] : hevc_enc_init : optional, when present and set to 'true' indicates that frame_type in hevc_enc_picture_t might come with value different than HEVC_ENC_FRAME_TYPE_AUTO  
*
* Additionally, hevc_enc_init_params_t structure will contain all plugin-specific properties set via XML interface (ACCESS_TYPE_USER).
*
*/

/** @brief Set of properties to init encoder */
typedef struct {
    const Property* properties; /**< Pointer to array of properties */
    size_t count;               /**< Number of properties in array */
} HevcEncInitParams;

/** @brief Color Space */
typedef enum {
    HEVC_ENC_COLOR_SPACE_I400 = 0,
    HEVC_ENC_COLOR_SPACE_I420,
    HEVC_ENC_COLOR_SPACE_I422,
    HEVC_ENC_COLOR_SPACE_I444,
} HevcEncColorSpace;

/** @brief Type of video frame */
typedef enum {
    HEVC_ENC_FRAME_TYPE_AUTO = 0,
    HEVC_ENC_FRAME_TYPE_IDR,
    HEVC_ENC_FRAME_TYPE_I,
    HEVC_ENC_FRAME_TYPE_P,
    HEVC_ENC_FRAME_TYPE_B,
    HEVC_ENC_FRAME_TYPE_BREF,
} HevcEncFrameType;

/** @brief Picture structure
 *  Points to video data and describes how it should be interpreted.
 */
typedef struct {
    int bitDepth;   /**< 8 or 10 bits */
    void* plane[3]; /**< Pointers to Y, Cb and Cr planes */
    int stride[3];  /**< Number of bytes between rows (for each plane) */

    size_t width;
    size_t height;
    HevcEncColorSpace colorSpace;
    HevcEncFrameType frameType;
} HevcEncPicture;

/** @brief NAL unit types.
 *         In DEE only HEVC_ENC_NAL_UNIT_ACCESS_UNIT_DELIMITER is important.
 *         All other NAL units can be set to HEVC_ENC_NAL_UNIT_OTHER.
 */
typedef enum {
    HEVC_ENC_NAL_UNIT_CODED_SLICE_TRAIL_N = 0,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_TRAIL_R,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_TSA_N,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_TLA_R,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_STSA_N,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_STSA_R,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_RADL_N,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_RADL_R,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_RASL_N,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_RASL_R,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_BLA_W_LP = 16,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_BLA_W_RADL,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_BLA_N_LP,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_IDR_W_RADL,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_IDR_N_LP,
    HEVC_ENC_NAL_UNIT_CODED_SLICE_CRA,
    HEVC_ENC_NAL_UNIT_VPS = 32,
    HEVC_ENC_NAL_UNIT_SPS,
    HEVC_ENC_NAL_UNIT_PPS,
    HEVC_ENC_NAL_UNIT_ACCESS_UNIT_DELIMITER,
    HEVC_ENC_NAL_UNIT_EOS,
    HEVC_ENC_NAL_UNIT_EOB,
    HEVC_ENC_NAL_UNIT_FILLER_DATA,
    HEVC_ENC_NAL_UNIT_PREFIX_SEI,
    HEVC_ENC_NAL_UNIT_SUFFIX_SEI,
    HEVC_ENC_NAL_UNIT_OTHER,
    HEVC_ENC_NAL_UNIT_INVALID = 64,
} HevcEncNalType;

/** @brief NAL unit structure */
typedef struct {
    HevcEncNalType type; /**< Type of NAL unit */
    size_t size;         /**< Size of payload */
    void* payload;       /**< Pointer to payload */
} HevcEncNal;

/** @brief Output frame structure */
typedef struct {
    HevcEncNal* nal; /**< Buffer of NAL units */
    size_t nalNum;   /**< Number of units in buffer */
} HevcEncOutput;

/** @brief Handle to encoder instance
 *  hevc_enc_handle_t is actually pointer to encoder context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* HevcEncHandle;

/** @brief Get info about supported properties
 *  @return Number of properties in info array
 */
typedef size_t (*HevcEncGetInfo)(const PropertyInfo** info); /**< [out] Pointer to array with property information */

/** @brief Get size of encoder context
 *  @return Size in bytes
 */
typedef size_t (*HevcEncGetSize)(void);

/** @brief Initialize encoder instance
 *  @return status code
 */
typedef Status (*HevcEncInit)(HevcEncHandle handle,                 /**< [in/out] Encoder instance handle */
                              const HevcEncInitParams* initParams); /**< [in] Properties to init encoder instance */

/** @brief Close encoder instance
 *  @return status code
 */
typedef Status (*HevcEncClose)(HevcEncHandle handle); /**< [in/out] Encoder instance handle */

/** @brief Encode array of pictures. In DEE picture_num is always '1'.
 *         Should produce complete access unit, if available.
 *         If buffer size (max_output_data) is too small to handle access unit,
 *         plugin must keep it until bigger buffer is available.
 *         Function sets pointer to output data, so it must
 *         stay in plugin's memory until next 'process' call.
 *  @return status code
 */
typedef Status (*HevcEncProcess)(HevcEncHandle handle,          /**< [in/out] Encoder instance handle */
                                 const HevcEncPicture* picture, /**< [in] Pointer to array of pictures */
                                 const size_t pictureNum,       /**< [in] Number of pictures in array */
                                 HevcEncOutput* output);        /**< [out] Output buffer */

/** @brief Flush encoder
 *  @return status code
 */
typedef Status (*HevcEncFlush)(HevcEncHandle handle,  /**< [in/out] Encoder instance handle */
                               HevcEncOutput* output, /**< [in/out] Output buffer */
                               int* isEmpty);         /**< [out] Flush indicator */

/** @brief Property setter
 *  @return status code
 */
typedef Status (*HevcEncSetProperty)(HevcEncHandle handle,      /**< [in/out] Encoder instance handle */
                                     const Property* property); /**< [in] Property to write */

/** @brief Property getter
 *  @return status code
 */
typedef Status (*HevcEncGetProperty)(HevcEncHandle handle, /**< [in/out] Encoder instance handle */
                                     Property* property);  /**< [in/out] Property to read */

/** @brief Get string describing last warning or error occurred
 *  @return Pointer to string with message
 */
typedef const char* (*HevcEncGetMessage)(HevcEncHandle handle); /**< [in/out] Encoder instance handle */

/** @brief Structure with pointers to all API functions */
typedef struct {
    const char* pluginName;
    HevcEncGetInfo getInfo;
    HevcEncGetSize getSize;
    HevcEncInit init;
    HevcEncClose close;
    HevcEncProcess process;
    HevcEncFlush flush;
    HevcEncSetProperty setProperty;
    HevcEncGetProperty getProperty;
    HevcEncGetMessage getMessage;
} HevcEncApi;

/** @brief Export symbol to access implementation of HEVC Encoder plugin
 *  @return pointer to hevc_enc_api_t
 */
DLB_EXPORT
HevcEncApi* hevcEncGetApi(void);

/** @brief Definition of pointer to hevcEncGetApi function */
typedef HevcEncApi* (*HevcEncGetApi)(void);

/** @brief Export symbol to access API version of HEVC Encoder plugin
*  @return integer representing API version
*/
DLB_EXPORT
int hevcEncGetApiVersion(void);

/** @brief Definition of pointer to hevcEncGetApiVersion function */
typedef int (*HevcEncGetApiVersion)(void);

#ifdef __cplusplus
}
#endif

#endif // __DEE_PLUGINS_HEVC_ENC_API_H__
