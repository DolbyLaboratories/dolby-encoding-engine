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

#ifndef __DEE_PLUGINS_COMMON_H__
#define __DEE_PLUGINS_COMMON_H__

#include <string.h>
 
/** @brief  Status code */
typedef enum
{
    STATUS_OK = 0,
    STATUS_WARNING,
    STATUS_ERROR
} status_t;
  
/** @brief  Property read/write structure */
typedef struct
{
    char*   name;           /**< Name of property */
    size_t  max_name_sz;    /**< Size of buffer pointed by 'name' */
    char*   value;          /**< Value of property */
    size_t  max_value_sz;   /**< Size of buffer pointed by 'value' */     
} property_t;
  
/** @brief Type o property
 *  Defines how property is presented in XML schema.
 */
typedef enum
{
    PROPERTY_TYPE_STRING = 0,
    PROPERTY_TYPE_INTEGER,
    PROPERTY_TYPE_DECIMAL,
    PROPERTY_TYPE_BOOLEAN
} property_type_t;

/** @brief Type of access to property */
typedef enum
{
    ACCESS_TYPE_WRITE = 0,
    ACCESS_TYPE_WRITE_INIT,     /**< Writable only during init */
    ACCESS_TYPE_READ,
    ACCESS_TYPE_RW,
    ACCESS_TYPE_USER,           /**< Means ACCESS_TYPE_WRITE_INIT + show in XML */
} access_type_t;
  
/** @brief  Property information structure */
typedef struct
{
    const char*     name;           /**< Name of property */
    property_type_t type;           /**< Type of property */
    const char*     description;    /**< Description of property */
    const char*     defval;         /**< Default value of property */
    const char*     values;         /**< List of supported values or range if numeric type */
    int             min_occurs;     /**< Minimal required number of occurrences */
    int             max_occurs;     /**< Maximal supported number of occurrences */
    access_type_t   access;         /**< Type of access to property */
} property_info_t;

#ifdef __cplusplus
#ifdef WIN32
#ifdef __GNUC__
#define DLB_EXPORT extern "C" __attribute__((dllexport))
#else
#define DLB_EXPORT extern "C" __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define DLB_EXPORT extern "C" __attribute__((visibility("default")))
#else
#define DLB_EXPORT extern "C"
#endif
#endif
#else
#ifdef WIN32
#ifdef __GNUC__
#define DLB_EXPORT __attribute__((dllexport))
#else
#define DLB_EXPORT __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define DLB_EXPORT __attribute__((visibility("default")))
#else
#define DLB_EXPORT
#endif
#endif
#endif

#endif /* __DEE_PLUGINS_COMMON_H__ */
