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

#ifndef __DEE_PLUGINS_HEVC_DEC_BEAMR_IMPL_H__
#define __DEE_PLUGINS_HEVC_DEC_BEAMR_IMPL_H__

#include <atomic>
#include <cstdio>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>

#include "debugger.h"
#include "hevc_dec_api.h"
#include "hevc_dec_common.h"

#include "vh3_decode.h"
#include "vh3_default_callbacks.h"

struct PluginCtrl {
    std::string msg;
    HevcDecPicture pic;

    std::string output_format{"any"};
    HevcDecFrameRate frame_rate{24, 1};
    bool use_sps_frame_rate{false};
    uint32_t flags {0};
    int output_delay{16};
    bool mt_disable{false};
    int mt_num_threads{4};
    int mt_num_wf_lines{4};
    int mt_num_frames{3};
    int mt_num_input_units{4};
    uint32_t mt_flags {3};
    uint64_t mt_aff_mask {0};
    uint32_t disable_cpu_extensions {0};
    int debug_level{0};

    PluginCtrl() {
        memset(&pic, 0, sizeof(pic));
    }
};

struct OutPictureData {
    const vh3_Picture* pic;
    vh3_RecPictureInfo picInfo;
};

// Forward declaration of callback functions
extern "C" {
vh3_rec_send_cb cb_rec_send;
vh3_ms_send_cb cb_ms_send;
vh3_notify_cb cb_notify;
}

class Decoder {
public:
    Decoder();
    virtual ~Decoder();
    std::string getVersion();
    std::string getMessage();
    void init(PluginCtrl& ctrl);
    void feed(uint8_t* bytes, uint64_t bytesNum);
    void flush();
    bool getPic(HevcDecPicture& outPic);
    void close();

    int debugLevel{0};

protected:
    static const size_t MAX_STRING_SIZE {4096};
    static const size_t MAX_NALU_SIZE {128 * 1024 * 1024};
    dee::HevcStream stream;
    std::string version{"Beamr SDK"};
    std::mutex dataLock;
    vh3_DecoderHandle hDec;
    hevcd_settings_t settings;
    std::queue<OutPictureData> qRec;
    uint8_t* outBuffer{nullptr};
    uint64_t outBufferSize{0};
    vh3_CallbacksTable_t cbTable;
    char stringBuffer[Decoder::MAX_STRING_SIZE + 1];
    std::list<std::string> pendingErrors;
    std::list<std::string> pendingMsgs;
    std::atomic_bool flushing{false};
    std::atomic_bool flushed{false};
    bool opened{false};
    int vuiNumUnitsInTick{-1};
    int vuiTimeScale{-1};
    bool useSpsFramerate{false};

    bool consumeStream();
    void sendToDecode(uint64_t startoffset, uint64_t endOffset);
    void checkPendingErrors();
    void checkErrorCode(const hevc_error_t errCode);
    void message(const char* fmt, ...);
    void error(const char* fmt, ...);
    void cropFrame(vh3_Picture* dst, const vh3_Picture* const src, const vh3_PictureDisplay& params);
    void writeFrame(const vh3_Picture* pic, HevcDecPicture& out);
    void flushDecodeThread();

    hevc_error_t notify(vh3_Notification a);
    hevc_error_t recSend(const vh3_Picture* pic, vh3_RecPictureInfo info);
    hevc_error_t msSend(const vh3_MediaSample* ms, vh3_NalInfo info);

    friend hevc_error_t VSSHSDKAPI cb_notify(void* ctx, vh3_Notification a);
    friend hevc_error_t VSSHSDKAPI cb_rec_send(void* ctx, const vh3_Picture* pic, vh3_RecPictureInfo info);
    friend hevc_error_t VSSHSDKAPI cb_ms_send(void* ctx, const vh3_MediaSample* ms, vh3_NalInfo info);
};

/* This structure can contain only pointers and simple types */
struct PluginContext {
    PluginCtrl* ctrl{nullptr};
    Decoder* decoder{nullptr};
};

#endif //__DEE_PLUGINS_HEVC_DEC_BEAMR_IMPL_H__
