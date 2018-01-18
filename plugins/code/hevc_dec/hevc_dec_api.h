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

#ifndef __DEE_PLUGINS_HEVC_DEC_API_H__
#define __DEE_PLUGINS_HEVC_DEC_API_H__

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Framework sets/gets following properties. Each hevc_dec plugin should handle them. 
*      PROPERTY : TYPE : VALUES : WHERE : COMMENT
* -----------------------------------------------------------------------------
*      max_output_data : integer : n/a : hevc_enc_set_property : framework indicates max size of buffer before each 'process' call
*
* Additionally, hevc_dec_init_params_t structure will contain all plugin-specific properties set via XML interface (ACCESS_TYPE_USER).
*
*/

/** @brief  Status code */
typedef enum
{
    HEVC_DEC_OK = 0,
    HEVC_DEC_PICTURE_NOT_READY,
    HEVC_DEC_WARNING,
    HEVC_DEC_ERROR
} hevc_dec_status_t;

/** @brief Set of properties to init encoder */
typedef struct
{
    const property_t* property;     /**< Pointer to array of properties */       
    size_t            count;        /**< Number of properties in array */
} hevc_dec_init_params_t;
 
/** @brief Frame Rate calculated as frame_period divided by time_scale */
typedef struct
{
    int frame_period;
    int time_scale;
} hevc_dec_frame_rate_t;

/** @brief Color Space */
typedef enum
{
    COLOR_SPACE_I400 = 0,
    COLOR_SPACE_I420,
    COLOR_SPACE_I422,
    COLOR_SPACE_I444
} hevc_dec_color_space_t;
 
/** @brief Type of video frame */
typedef enum
{
    FRAME_TYPE_AUTO = 0,
    FRAME_TYPE_IDR,
    FRAME_TYPE_I,
    FRAME_TYPE_P,
    FRAME_TYPE_B,
    FRAME_TYPE_BREF
} hevc_dec_frame_type_t;
 
/** @brief Picture structure
 *  Points to video data and describes how it should be interpreted.
 */
typedef struct
{
    size_t                  width;     
    size_t                  height;
    int                     bit_depth;      /**< 8 or 10 bits */
    hevc_dec_color_space_t  color_space;
    hevc_dec_frame_type_t   frame_type;
    hevc_dec_frame_rate_t   frame_rate;
    size_t                  luma_size;      /**< Y plane data size in bytes */
    size_t                  chroma_size;    /**< Cb and Cr plane data size in bytes */
    int                     transfer_characteristics;
    int                     matrix_coeffs;
    void*                   plane[3];       /**< Pointers to Y, Cb and Cr planes */
    int                     stride[3];      /**< Number of bytes between rows (for each plane) */
} hevc_dec_picture_t;
 
/** @brief Handle to decoder instance
 *  hevc_dec_handle_t is actually pointer to decoder context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* hevc_dec_handle_t;
 
/** @brief Get info about supported properties
 *  @return Number of properties in info array
 */
size_t
hevc_dec_get_info
    (const property_info_t** info    /**< [out] Pointer to array with property information */
    );
     
/** @brief Definition of pointer to hevc_dec_get_info function */
typedef size_t (*func_hevc_dec_get_info)(const property_info_t**);
 
/** @brief Get size of decoder context
 *  @return Size in bytes
 */
size_t
hevc_dec_get_size();
 
/** @brief Definition of pointer to hevc_dec_get_size function */
typedef size_t (*func_hevc_dec_get_size)();
 
/** @brief Initialize decoder instance
 *  @return status code 
 */
hevc_dec_status_t
hevc_dec_init
    (hevc_dec_handle_t              handle          /**< [in/out] Decoder instance handle */
    ,const hevc_dec_init_params_t*  init_params     /**< [in] Properties to init decoder instance */
    );
     
/** @brief Definition of pointer to hevc_dec_init function */  
typedef hevc_dec_status_t(*func_hevc_dec_init)(hevc_dec_handle_t, const hevc_dec_init_params_t*);
 
/** @brief Close decoder instance 
 *  @return status code
 */
hevc_dec_status_t
hevc_dec_close
    (hevc_dec_handle_t handle   /**< [in/out] Decoder instance handle */
    );
 
/** @brief Definition of pointer to hevc_dec_close function */ 
typedef hevc_dec_status_t(*func_hevc_dec_close)(hevc_dec_handle_t);
 
/** @brief pass a chunk of bitsteam data to decoder and retrieve one decoded picture if available
 *  @return status code
 */
hevc_dec_status_t
hevc_dec_process
    (hevc_dec_handle_t          handle          /**< [in/out] Encoder instance handle */ 
    ,const void*                stream_buffer   /**< [in] Pointer to buffer containing hevc stream */
    ,const size_t               buffer_size     /**< [in] Size of hevc buffer */
    ,hevc_dec_picture_t*        output          /**< [out] Output buffer */
    );
     
/** @brief Definition of pointer to hevc_dec_process function */
typedef hevc_dec_status_t(*func_hevc_dec_process)(hevc_dec_handle_t, const void*, const size_t, hevc_dec_picture_t*);
 
/** @brief Flush decoder
 *  @return status code
 */
hevc_dec_status_t
hevc_dec_flush
    (hevc_dec_handle_t      handle          /**< [in/out] Decoder instance handle */
    ,hevc_dec_picture_t*     output          /**< [in/out] Output buffer */
    ,int*                   is_empty        /**< [out] Flush indicator */
    );
 
/** @brief Definition of pointer to hevc_dec_flush function */
typedef hevc_dec_status_t(*func_hevc_dec_flush)(hevc_dec_handle_t, hevc_dec_picture_t*, int*);
 
/** @brief Property setter
 *  @return status code
 */ 
hevc_dec_status_t
hevc_dec_set_property
    (hevc_dec_handle_t handle       /**< [in/out] Decoder instance handle */
    ,const property_t* property     /**< [in] Property to write */
    ); 
 
/** @brief Definition of pointer to hevc_dec_set_property function */
typedef hevc_dec_status_t(*func_hevc_dec_set_property)(hevc_dec_handle_t, const property_t*);
 
/** @brief Property getter
 *  @return status code
 */ 
hevc_dec_status_t
hevc_dec_get_property
    (hevc_dec_handle_t handle       /**< [in/out] Decoder instance handle */
    ,property_t* property           /**< [in/out] Property to read */
    );
     
/** @brief Definition of pointer to hevc_dec_get_property function */  
typedef hevc_dec_status_t(*func_hevc_dec_get_property)(hevc_dec_handle_t, property_t*);
 
/** @brief Get string describing last warning or error occurred
 *  @return Pointer to string with message
 */ 
const char*
hevc_dec_get_message
    (hevc_dec_handle_t handle          /**< [in/out] Decoder instance handle */
    );
     
/** @brief Definition of pointer to hevc_dec_get_property function */  
typedef const char* (*func_hevc_dec_get_message)(hevc_dec_handle_t);
 
/** @brief Structure with pointers to all API functions */
typedef struct
{
    const char*                        plugin_name;                   
    func_hevc_dec_get_info             get_info;
    func_hevc_dec_get_size             get_size;
    func_hevc_dec_init                 init;
    func_hevc_dec_close                close;
    func_hevc_dec_process              process;
    func_hevc_dec_flush                flush;
    func_hevc_dec_set_property         set_property;
    func_hevc_dec_get_property         get_property;
    func_hevc_dec_get_message          get_message;
} hevc_dec_api_t;
 
/** @brief Export symbol to access implementation of HEVC Decoder plugin
 *  @return pointer to hevc_dec_api_t
 */
DLB_EXPORT
hevc_dec_api_t* hevc_dec_get_api();
 
/** @brief Definition of pointer to hevc_dec_get_api function */
typedef 
hevc_dec_api_t* (*func_hevc_dec_get_api)();

#ifdef __cplusplus
}
#endif

#endif // __DEE_PLUGINS_HEVC_DEC_API_H__
