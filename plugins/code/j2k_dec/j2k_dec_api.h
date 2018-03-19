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

#ifndef __DEE_PLUGINS_J2K_DEC_API_H__
#define __DEE_PLUGINS_J2K_DEC_API_H__

#include "common.h"

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
* Additionally, j2k_dec_init_params_t structure will contain all plugin-specific properties set via XML interface (ACCESS_TYPE_USER).
*
*/

/** @brief Set of properties to init decoder */
typedef struct
{
    const property_t* property;     /**< Pointer to array of properties */       
    size_t            count;        /**< Number of properties in array */
} j2k_dec_init_params_t;

/** @brief j2k_dec input */
typedef struct
{
    void*   buffer;     /**< Pointer to J2K frame */
    size_t  size;       /**< Size of J2K frame */
} j2k_dec_input_t;

/** @brief j2k_dec output */
typedef struct
{
    size_t      width;      /**< Width of decoded picture */
    size_t      height;     /**< Height of decoded picture */
    void*       buffer[3];  /**< Pointers to RGB planes, 16 bits per pixel */
} j2k_dec_output_t;

/** @brief Handle to decoder instance
 *  j2k_dec_handle_t is actually pointer to decoder context
 *  defined by plugin's private type. Caller sees that as void*.
 */
typedef void* j2k_dec_handle_t;

/** @brief Get info about supported properties
 *  @return Number of properties in info array
 */
size_t
j2k_dec_get_info
    (const property_info_t** info    /**< [out] Pointer to array with property information */
    );

/** @brief Definition of pointer to j2k_dec_get_info function */
typedef size_t (*func_j2k_dec_get_info)(const property_info_t**);

/** @brief Get size of decoder context
 *  @return Size in bytes
 */
size_t
j2k_dec_get_size(void);
 
/** @brief Definition of pointer to j2k_dec_get_size function */
typedef size_t (*func_j2k_dec_get_size)(void);

/** @brief Initialize decoder instance
 *  @return status code 
 */
status_t
j2k_dec_init
    (j2k_dec_handle_t               handle          /**< [in/out] Decoder instance handle */
    ,const j2k_dec_init_params_t*   init_params     /**< [in] Properties to init decoder instance */
    );
     
/** @brief Definition of pointer to j2k_dec_init function */  
typedef status_t (*func_j2k_dec_init)(j2k_dec_handle_t, const j2k_dec_init_params_t*);

/** @brief Close decoder instance 
 *  @return status code
 */
status_t
j2k_dec_close
    (j2k_dec_handle_t handle   /**< [in/out] Decoder instance handle */
    );

/** @brief Definition of pointer to j2k_dec_close function */ 
typedef status_t (*func_j2k_dec_close)(j2k_dec_handle_t);

/** @brief Decode j2k picture. Each call must produce decoded picture.
 *  @return status code
 */
status_t
j2k_dec_process
    (j2k_dec_handle_t               handle  /**< [in/out] Decoder instance handle */ 
    ,const j2k_dec_input_t*     in      /**< [in] Encoded input */
    ,j2k_dec_output_t*          out     /**< [out] Decoded output */
    );

/** @brief Definition of pointer to j2k_dec_process function */
typedef status_t (*func_j2k_dec_process)(j2k_dec_handle_t, const j2k_dec_input_t*, j2k_dec_output_t*);

/** @brief Property setter
 *  @return status code
 */ 
status_t
j2k_dec_set_property
    (j2k_dec_handle_t handle        /**< [in/out] Decoder instance handle */
    ,const property_t* property     /**< [in] Property to write */
    ); 

/** @brief Definition of pointer to j2k_dec_set_property function */
typedef status_t (*func_j2k_dec_set_property)(j2k_dec_handle_t, const property_t*);

/** @brief Property getter
 *  @return status code
 */ 
status_t
j2k_dec_get_property
    (j2k_dec_handle_t handle        /**< [in/out] Decoder instance handle */
    ,property_t* property           /**< [in/out] Property to read */
    );

/** @brief Definition of pointer to j2k_dec_get_property function */  
typedef status_t (*func_j2k_dec_get_property)(j2k_dec_handle_t, property_t*);

/** @brief Get string describing last warning or error occurred
 *  @return Pointer to string with message
 */ 
const char*
j2k_dec_get_message
    (j2k_dec_handle_t handle        /**< [in/out] Decoder instance handle */
    );
     
/** @brief Definition of pointer to j2k_dec_get_message function */  
typedef const char* (*func_j2k_dec_get_message)(j2k_dec_handle_t);

/** @brief Structure with pointers to all API functions */
typedef struct
{
    const char*                     plugin_name;                   
    func_j2k_dec_get_info           get_info;
    func_j2k_dec_get_size           get_size; 
    func_j2k_dec_init               init;
    func_j2k_dec_close              close;
    func_j2k_dec_process            process;
    func_j2k_dec_set_property       set_property;
    func_j2k_dec_get_property       get_property;
    func_j2k_dec_get_message        get_message;
} j2k_dec_api_t;
 
/** @brief Export symbol to access implementation of J2K Decoder plugin
 *  @return pointer to j2k_dec_api_t
 */
DLB_EXPORT
j2k_dec_api_t* j2k_dec_get_api(void);
 
/** @brief Definition of pointer to j2k_dec_get_api function */
typedef 
j2k_dec_api_t* (*func_j2k_dec_get_api)(void);

#ifdef __cplusplus
}
#endif

#endif // __DEE_PLUGINS_J2K_DEC_API_H__
