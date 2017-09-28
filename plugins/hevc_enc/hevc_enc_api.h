/*
* BSD 3-Clause License
*
* Copyright (c) 2017, Dolby Laboratories
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

#ifdef __cplusplus
extern "C" {
#endif

/* Framework sets/gets following properties. Each hevc_enc plugin should handle them. 
*      PROPERTY : TYPE : VALUES : WHERE : COMMENT
* -----------------------------------------------------------------------------
*      width : integer : n/a : hevc_enc_init
*      height : integer : n/a : hevc_enc_init
*      bit_depth : enum : [8,10] : hevc_enc_init : in DEE 1.0 it is always '10'
*      frame_rate : enum : [23.976,24,25,29.97,30,48,59.94,60] : hevc_enc_init 
*      color_space : enum : [i400,i420,i422,i444] : hevc_enc_init : in DEE 1.0 it is always 'i420'
*      data_rate : integer : n/a : hevc_enc_init : value in kbps
*      max_vbv_data_rate : integer : n/a : hevc_enc_init : value in kbps
*      vbv_buffer_size : integer : n/a : hevc_enc_init : value in kb
*      range : enum : [limited,full] : hevc_enc_init : in DEE 1.0 it is always 'full'
*      multi_pass : enum : [off,1st,nth,last] : hevc_enc_init
*      stats_file : string : n/a : hevc_enc_init : temp file to write stats in multipass encoding
*      max_output_data : integer : n/a : hevc_enc_set_property : framework indicates max size of buffer before each 'process' call
*      max_pass_num : integer : n/a : hevc_enc_get_property : accessed before hevc_enc_init
*      temp_file_num : integer : n/a : hevc_enc_get_property : accessed before hevc_enc_init, optional, should be used if plugin needs some temp files
*      temp_file : string : n/a : hevc_enc_init : occurs multiple times, according to value retrieved from 'temp_file_num'
*
* Additionally, hevc_enc_init_params_t structure will contain all plugin-specific properties set via XML interface (ACCESS_TYPE_USER).
*
*/

/** @brief Set of properties to init encoder */
typedef struct
{
    const property_t* property;     /**< Pointer to array of properties */       
    size_t            count;        /**< Number of properties in array */
} hevc_enc_init_params_t;
 
/** @brief Color Space */
typedef enum
{
    COLOR_SPACE_I400 = 0,
    COLOR_SPACE_I420,
    COLOR_SPACE_I422,
    COLOR_SPACE_I444
} color_space_t;
 
/** @brief Type of video frame */
typedef enum
{
    FRAME_TYPE_AUTO = 0,
    FRAME_TYPE_IDR,
    FRAME_TYPE_I,
    FRAME_TYPE_P,
    FRAME_TYPE_B,
    FRAME_TYPE_BREF
} frame_type_t;
 
/** @brief Picture structure
 *  Points to video data and describes how it should be interpreted.
 */
typedef struct
{
    size_t          width;     
    size_t          height;
    int             bit_depth;      /**< 8 or 10 bits */
    color_space_t   color_space;   
    frame_type_t    frame_type;
    void*           plane[3];       /**< Pointers to Y, Cb and Cr planes */
    int             stride[3];      /**< Number of bytes between rows (for each plane) */
} hevc_enc_picture_t;

/** @brief NAL unit types.
 *         In DEE 1.0 only HEVC_ENC_NAL_UNIT_ACCESS_UNIT_DELIMITER is important.
 *         All other NAL units can be set to HEVC_ENC_NAL_UNIT_OTHER.
 */
typedef enum
{
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
} hevc_enc_nal_type_t;

/** @brief NAL unit structure */
typedef struct
{
    hevc_enc_nal_type_t type;       /**< Type of NAL unit */
    size_t              size;       /**< Size of payload */
    void*               payload;    /**< Pointer to payload */
} hevc_enc_nal_t;

/** @brief Output frame structure */
typedef struct
{
    hevc_enc_nal_t* nal;        /**< Buffer of NAL units */       
    size_t          nal_num;    /**< Number of units in buffer */
} hevc_enc_output_t;
 
/** @brief Handle to encoder instance
 *  hevc_enc_handle_t is actually pointer to encoder context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* hevc_enc_handle_t;
 
/** @brief Get info about supported properties
 *  @return Number of properties in info array
 */
size_t
hevc_enc_get_info
    (const property_info_t** info    /**< [out] Pointer to array with property information */
    );
     
/** @brief Definition of pointer to hevc_enc_get_info function */
typedef size_t (*func_hevc_enc_get_info)(const property_info_t**);
 
/** @brief Get size of encoder context
 *  @return Size in bytes
 */
size_t
hevc_enc_get_size();
 
/** @brief Definition of pointer to hevc_enc_get_size function */
typedef size_t (*func_hevc_enc_get_size)();
 
/** @brief Initialize encoder instance
 *  @return status code 
 */
status_t
hevc_enc_init
    (hevc_enc_handle_t              handle          /**< [in/out] Encoder instance handle */
    ,const hevc_enc_init_params_t*  init_params     /**< [in] Properties to init encoder instance */
    );
     
/** @brief Definition of pointer to hevc_enc_init function */  
typedef status_t (*func_hevc_enc_init)(hevc_enc_handle_t, const hevc_enc_init_params_t*);
 
/** @brief Close encoder instance 
 *  @return status code
 */
status_t
hevc_enc_close
    (hevc_enc_handle_t handle   /**< [in/out] Encoder instance handle */
    );
 
/** @brief Definition of pointer to hevc_enc_close function */ 
typedef status_t (*func_hevc_enc_close)(hevc_enc_handle_t);
 
/** @brief Encode array of pictures. In DEE 1.0 picture_num is always '1'.
 *         Should produce complete access unit, if available.
 *         If buffer size (max_output_data) is too small to handle access unit,
 *         plugin must keep it until bigger buffer is available.
 *         Function sets pointer to output data, so it must 
 *         stay in plugin's memory until next 'process' call.
 *  @return status code
 */
status_t
hevc_enc_process
    (hevc_enc_handle_t          handle      /**< [in/out] Encoder instance handle */ 
    ,const hevc_enc_picture_t*  picture     /**< [in] Pointer to array of pictures */
    ,const size_t               picture_num /**< [in] Number of pictures in array */
    ,hevc_enc_output_t*         output      /**< [out] Output buffer */
    );
     
/** @brief Definition of pointer to hevc_enc_process function */
typedef status_t (*func_hevc_enc_process)(hevc_enc_handle_t, const hevc_enc_picture_t*, const size_t, hevc_enc_output_t*);
 
/** @brief Flush encoder
 *  @return status code
 */
status_t
hevc_enc_flush
    (hevc_enc_handle_t      handle          /**< [in/out] Encoder instance handle */
    ,hevc_enc_output_t*     output          /**< [in/out] Output buffer */
    ,int*                   is_empty        /**< [out] Flush indicator */
    );
 
/** @brief Definition of pointer to hevc_enc_flush function */
typedef status_t (*func_hevc_enc_flush)(hevc_enc_handle_t, hevc_enc_output_t*, int*);
 
/** @brief Property setter
 *  @return status code
 */ 
status_t
hevc_enc_set_property
    (hevc_enc_handle_t handle       /**< [in/out] Encoder instance handle */
    ,const property_t* property     /**< [in] Property to write */
    ); 
 
/** @brief Definition of pointer to hevc_enc_set_property function */
typedef status_t (*func_hevc_enc_set_property)(hevc_enc_handle_t, const property_t*);
 
/** @brief Property getter
 *  @return status code
 */ 
status_t
hevc_enc_get_property
    (hevc_enc_handle_t handle       /**< [in/out] Encoder instance handle */
    ,property_t* property           /**< [in/out] Property to read */
    );
     
/** @brief Definition of pointer to hevc_enc_get_property function */  
typedef status_t (*func_hevc_enc_get_property)(hevc_enc_handle_t, property_t*);
 
/** @brief Get string describing last warning or error occurred
 *  @return Pointer to string with message
 */ 
const char*
hevc_enc_get_message
    (hevc_enc_handle_t handle          /**< [in/out] Encoder instance handle */
    );
     
/** @brief Definition of pointer to hevc_enc_get_property function */  
typedef const char* (*func_hevc_enc_get_message)(hevc_enc_handle_t);
 
/** @brief Structure with pointers to all API functions */
typedef struct
{
    const char*                        plugin_name;                   
    func_hevc_enc_get_info             get_info;
    func_hevc_enc_get_size             get_size; 
    func_hevc_enc_init                 init;
    func_hevc_enc_close                close;
    func_hevc_enc_process              process;
    func_hevc_enc_flush                flush;
    func_hevc_enc_set_property         set_property;
    func_hevc_enc_get_property         get_property;
    func_hevc_enc_get_message          get_message;
} hevc_enc_api_t;
 
/** @brief Export symbol to access implementation of HEVC Encoder plugin
 *  @return pointer to hevc_enc_api_t
 */
DLB_EXPORT
hevc_enc_api_t* hevc_enc_get_api();
 
/** @brief Definition of pointer to hevc_enc_get_api function */
typedef 
hevc_enc_api_t* (*func_hevc_enc_get_api)();

#ifdef __cplusplus
}
#endif

#endif // __DEE_PLUGINS_HEVC_ENC_API_H__
