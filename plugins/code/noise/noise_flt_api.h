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

#ifndef __DLB_PLUGINS_NOISE_FLT_API_H__
#define __DLB_PLUGINS_NOISE_FLT_API_H__

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Framework sets/gets following properties. Each noise plugin should handle them. 
*      PROPERTY : TYPE : VALUES : WHERE : COMMENT
* -----------------------------------------------------------------------------
*      source_width : integer : n/a : noise_flt_init
*      source_height : integer : n/a : noise_flt_init
*      temp_file_num : integer : n/a : noise_flt_get_property : accessed before noise_flt_init, optional, should be used if plugin needs some temp files
*      temp_file : string : n/a : noise_flt_init : occurs multiple times, according to value retrieved from 'temp_file_num'
*
* Additionally, noise_flt_init_params_t structure will contain all plugin-specific properties set via XML interface (ACCESS_TYPE_USER).
*
*/

/** @brief Set of properties to init filter */
typedef struct
{
    const property_t* property;     /**< Pointer to array of properties */       
    size_t            count;        /**< Number of properties in array */
} noise_flt_init_params_t;

/** @brief noise_flt input/output picture */
typedef struct
{
    size_t      width;      /**< Width of picture */
    size_t      height;     /**< Height of picture */
    void*       buffer[3];  /**< Pointers to RGB planes, 16 bits per pixel */
} noise_flt_pic_t;

/** @brief Handle to filter instance
 *  noise_flt_handle_t is actually pointer to filter context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* noise_flt_handle_t;

/** @brief Get info about supported properties
 *  @return Number of properties in info array
 */
size_t
noise_flt_get_info
    (const property_info_t** info    /**< [out] Pointer to array with property information */
    );

/** @brief Definition of pointer to noise_flt_get_info function */
typedef size_t (*func_noise_flt_get_info)(const property_info_t**);

/** @brief Get size of filter context
 *  @return Size in bytes
 */
size_t
noise_flt_get_size();
 
/** @brief Definition of pointer to noise_flt_get_size function */
typedef size_t (*func_noise_flt_get_size)();

/** @brief Initialize filter instance
 *  @return status code 
 */
status_t
noise_flt_init
    (noise_flt_handle_t               handle          /**< [in/out] filter instance handle */
    ,const noise_flt_init_params_t*   init_params     /**< [in] Properties to init filter instance */
    );
     
/** @brief Definition of pointer to noise_flt_init function */  
typedef status_t (*func_noise_flt_init)(noise_flt_handle_t, const noise_flt_init_params_t*);

/** @brief Close filter instance 
 *  @return status code
 */
status_t
noise_flt_close
    (noise_flt_handle_t handle   /**< [in/out] filter instance handle */
    );

/** @brief Definition of pointer to noise_flt_close function */ 
typedef status_t (*func_noise_flt_close)(noise_flt_handle_t);

/** @brief Process picture
 *  @return status code
 */
status_t
noise_flt_process
    (noise_flt_handle_t        handle  /**< [in/out] filter instance handle */ 
    ,const noise_flt_pic_t*    in      /**< [in] Input picture */
    ,noise_flt_pic_t*          out     /**< [out] Output picture */
    );

/** @brief Definition of pointer to noise_flt_process function */
typedef status_t (*func_noise_flt_process)(noise_flt_handle_t, const noise_flt_pic_t*, noise_flt_pic_t*);

/** @brief Property setter
 *  @return status code
 */ 
status_t
noise_flt_set_property
    (noise_flt_handle_t handle      /**< [in/out] filter instance handle */
    ,const property_t* property     /**< [in] Property to write */
    ); 

/** @brief Definition of pointer to noise_flt_set_property function */
typedef status_t (*func_noise_flt_set_property)(noise_flt_handle_t, const property_t*);

/** @brief Property getter
 *  @return status code
 */ 
status_t
noise_flt_get_property
    (noise_flt_handle_t handle      /**< [in/out] filter instance handle */
    ,property_t* property           /**< [in/out] Property to read */
    );

/** @brief Definition of pointer to noise_flt_get_property function */  
typedef status_t (*func_noise_flt_get_property)(noise_flt_handle_t, property_t*);

/** @brief Get string describing last warning or error occurred
 *  @return Pointer to string with message
 */ 
const char*
noise_flt_get_message
    (noise_flt_handle_t handle        /**< [in/out] filter instance handle */
    );
     
/** @brief Definition of pointer to noise_flt_get_message function */  
typedef const char* (*func_noise_flt_get_message)(noise_flt_handle_t);

/** @brief Structure with pointers to all API functions */
typedef struct
{
    const char*                     plugin_name;                   
    func_noise_flt_get_info           get_info;
    func_noise_flt_get_size           get_size; 
    func_noise_flt_init               init;
    func_noise_flt_close              close;
    func_noise_flt_process            process;
    func_noise_flt_set_property       set_property;
    func_noise_flt_get_property       get_property;
    func_noise_flt_get_message        get_message;
} noise_flt_api_t;
 
/** @brief Export symbol to access implementation of Noise Filter plugin
 *  @return pointer to noise_flt_api_t
 */
DLB_EXPORT
noise_flt_api_t* noise_flt_get_api();
 
/** @brief Definition of pointer to noise_flt_get_api function */
typedef 
noise_flt_api_t* (*func_noise_flt_get_api)();

#ifdef __cplusplus
}
#endif

#endif // __DLB_PLUGINS_NOISE_FLT_API_H__