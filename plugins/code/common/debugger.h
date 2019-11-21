/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2019, Dolby Laboratories
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

#ifndef __DEE_PLUGINS_DEBUGGER_H__
#define __DEE_PLUGINS_DEBUGGER_H__

void prologue(void* pCtx, const char* func, const char* vaargs, const char* parent, unsigned int line);

template <typename RetvalType>
void epilogue(
    void* pCtx, const char* func, const char* vaargs, const char* parent, unsigned int line, RetvalType retval);

// With args
#define FUNCTION(func, ...)                                         \
    prologue(nullptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__); \
    func(__VA_ARGS__);                                              \
    epilogue(nullptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__, nullptr);

#define FUNCTION_RETVALA(retval, func, ...)                         \
    prologue(nullptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__); \
    auto retval = func(__VA_ARGS__);                                \
    epilogue(nullptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__, retval);

#define FUNCTION_RETVAL(retval, func, ...)                          \
    prologue(nullptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__); \
    retval = func(__VA_ARGS__);                                     \
    epilogue(nullptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__, retval);

#define FUNCTION_T(func, ...)                                    \
    prologue(this, #func, #__VA_ARGS__, __FUNCTION__, __LINE__); \
    func(__VA_ARGS__);                                           \
    epilogue(this, #func, #__VA_ARGS__, __FUNCTION__, __LINE__, nullptr);

#define FUNCTION_T_RETVALA(retval, func, ...)                    \
    prologue(this, #func, #__VA_ARGS__, __FUNCTION__, __LINE__); \
    auto retval = func(__VA_ARGS__);                             \
    epilogue(this, #func, #__VA_ARGS__, __FUNCTION__, __LINE__, retval);

#define FUNCTION_T_RETVAL(retval, func, ...)                     \
    prologue(this, #func, #__VA_ARGS__, __FUNCTION__, __LINE__); \
    retval = func(__VA_ARGS__);                                  \
    epilogue(this, #func, #__VA_ARGS__, __FUNCTION__, __LINE__, retval);

#define FUNCTION_P(ptr, func, ...)                              \
    prologue(ptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__); \
    func(__VA_ARGS__);                                          \
    epilogue(ptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__, nullptr);

#define FUNCTION_P_RETVALA(ptr, retval, func, ...)              \
    prologue(ptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__); \
    auto retval = func(__VA_ARGS__);                            \
    epilogue(ptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__, retval);

#define FUNCTION_P_RETVAL(ptr, retval, func, ...)               \
    prologue(ptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__); \
    retval = func(__VA_ARGS__);                                 \
    epilogue(ptr, #func, #__VA_ARGS__, __FUNCTION__, __LINE__, retval);

// Without without
#define FUNCTIONV(func)                                   \
    prologue(nullptr, #func, "", __FUNCTION__, __LINE__); \
    func();                                               \
    epilogue(nullptr, #func, "", __FUNCTION__, __LINE__, nullptr);

#define FUNCTIONV_RETVALA(retval, func)                   \
    prologue(nullptr, #func, "", __FUNCTION__, __LINE__); \
    auto retval = func();                                 \
    epilogue(nullptr, #func, "", __FUNCTION__, __LINE__, retval);

#define FUNCTIONV_RETVAL(retval, func)                    \
    prologue(nullptr, #func, "", __FUNCTION__, __LINE__); \
    retval = func();                                      \
    epilogue(nullptr, #func, "", __FUNCTION__, __LINE__, retval);

#define FUNCTIONV_T(func)                              \
    prologue(this, #func, "", __FUNCTION__, __LINE__); \
    func();                                            \
    epilogue(this, #func, "", __FUNCTION__, __LINE__, nullptr);

#define FUNCTIONV_T_RETVALA(retval, func)              \
    prologue(this, #func, "", __FUNCTION__, __LINE__); \
    auto retval = func();                              \
    epilogue(this, #func, "", __FUNCTION__, __LINE__, retval);

#define FUNCTIONV_T_RETVAL(retval, func)               \
    prologue(this, #func, "", __FUNCTION__, __LINE__); \
    retval = func();                                   \
    epilogue(this, #func, "", __FUNCTION__, __LINE__, retval);

#define FUNCTIONV_P(ptr, func)                        \
    prologue(ptr, #func, "", __FUNCTION__, __LINE__); \
    func();                                           \
    epilogue(ptr, #func, "", __FUNCTION__, __LINE__, nullptr);

#define FUNCTIONV_P_RETVALA(ptr, retval, func)        \
    prologue(ptr, #func, "", __FUNCTION__, __LINE__); \
    auto retval = func();                             \
    epilogue(ptr, #func, "", __FUNCTION__, __LINE__, retval);

#define FUNCTIONV_P_RETVAL(ptr, retval, func)         \
    prologue(ptr, #func, "", __FUNCTION__, __LINE__); \
    retval = func();                                  \
    epilogue(ptr, #func, "", __FUNCTION__, __LINE__, retval);

#endif //__DEE_PLUGINS_DEBUGGER_H__
