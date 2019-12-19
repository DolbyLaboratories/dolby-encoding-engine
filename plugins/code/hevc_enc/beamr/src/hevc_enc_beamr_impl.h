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

#ifndef __DEE_PLUGINS_HEVC_ENC_BEAMR_IMPL_H__
#define __DEE_PLUGINS_HEVC_ENC_BEAMR_IMPL_H__

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "debugger.h"
#include "hevc_enc_api.h"
#include "hevc_enc_beamr_utils.h"

#include "vh3_default_callbacks.h"
#include "vh3_encode.h"

struct PluginCtrl {
    std::string msg;

    std::string config_path;
    std::deque<std::string> temp_file;
    uint64_t max_output_data{0};
    uint8_t bit_depth;
    uint16_t width;
    uint16_t height;
    std::string color_space{"i420"};
    std::string frame_rate;
    unsigned int data_rate{15000};
    unsigned int max_vbv_data_rate{15000};
    unsigned int vbv_buffer_size{30000};
    std::string range{"full"};
    std::string multi_pass{"off"};
    std::string stats_file;
    std::string color_primaries;
    std::string transfer_characteristics;
    std::string matrix_coefficients;
    int chromaloc{0};
    int64_t mastering_display_sei_x1{-1};
    int64_t mastering_display_sei_y1{-1};
    int64_t mastering_display_sei_x2{-1};
    int64_t mastering_display_sei_y2{-1};
    int64_t mastering_display_sei_x3{-1};
    int64_t mastering_display_sei_y3{-1};
    int64_t mastering_display_sei_wx{-1};
    int64_t mastering_display_sei_wy{-1};
    int64_t mastering_display_sei_max_lum{-1};
    int64_t mastering_display_sei_min_lum{-1};
    int64_t light_level_max_content{-1};
    int64_t light_level_max_frame_average{-1};
    bool force_slice_type{false};
    bool uhd_bd{false};
    std::string preset{"medium"};
    std::string modifier;
    uint16_t gop_intra_period{32};
    uint16_t gop_idr_period{1};
    uint8_t gop_min_intra_period{4};
    uint8_t gop_minigop_size{8};
    uint8_t gop_min_minigop_size{8};
    uint8_t gop_max_refs{4};
    uint8_t me_scene_change{0};
    int32_t pps_cb_qp_offset{0};
    int32_t pps_cr_qp_offset{0};
    std::list<std::string> param;
    std::string native_config_file;
    std::string gop_structure_in_file;
    std::string gop_structure_out_file;
    int debug_level{0};
};

// Forward declaration of callback functions
extern "C" {
vh3_rec_send_cb cb_rec_send;
vh3_ms_send_cb cb_ms_send;
vh3_notify_cb cb_notify;
void cb_settings_change(void* ctx, const char* msg);
}

struct vh3_Nal {
    vh3_NalInfo info;
    const vh3_MediaSample* ms;
};

struct SliceInfo {
    int64_t index{-1};
    vh3_SliceType type{VH3_SLICE_UNKNOWN};
    int8_t idr_flag{0};
};

class Encoder {
public:
    Encoder();
    virtual ~Encoder();
    std::string getVersion();
    std::string getMessage();
    void init(PluginCtrl& ctrl);
    void feed(const HevcEncPicture* in);
    void flush();
    void getNal(HevcEncOutput* out, uint64_t maxSize);
    void close();

    int debugLevel{0};

protected:
    static const size_t MAX_STRING_SIZE{4096};
    std::string version{"Beamr SDK"};
    std::mutex dataLock;
    vh3_EncoderHandle hEnc;
    hevce_settings_t settings;
    std::deque<vh3_Nal> qMS;
    std::vector<HevcEncNal> nal;
    uint8_t* outBuffer{nullptr};
    uint64_t outBufferSize{0};
    uint64_t nalUnitsToRelease{0};
    vh3_CallbacksTable_t cbTable;
    char stringBuffer[Encoder::MAX_STRING_SIZE + 1];
    std::list<std::string> pendingErrors;
    std::list<std::string> pendingMsgs;
    bool flushing{false};
    bool opened{false};
    int vuiNumUnitsInTick{-1};
    int vuiTimeScale{-1};
    bool useSpsFramerate{false};
    uint64_t frameIndex{0};
    uint64_t recFrameIndex{0};
    std::string dualPassFileName;
    std::vector<char> dualPassBuffer;
    std::ifstream gopStructureIn;
    std::ofstream gopStructureOut;
    std::deque<SliceInfo> slices;
    std::deque<SliceInfo> gopSlices;

    void checkPendingErrors();
    void checkErrorCode(const hevc_error_t errCode);
    void message(const char* fmt, ...);
    void message(const std::string& s);
    void error(const char* fmt, ...);
    void readFrame(const HevcEncPicture* in, vh3_Picture* pic);
    void releaseNalUnits();
    void parseGopStructureFile();
    void reportSliceInfo(vh3_RecPictureInfo* info);

    hevc_error_t notify(vh3_Notification a);
    hevc_error_t recSend(const vh3_Picture* pic, vh3_RecPictureInfo info);
    hevc_error_t msSend(const vh3_MediaSample* ms, vh3_NalInfo info);
    void settingsChange(const char* msg);

    friend hevc_error_t VSSHSDKAPI cb_notify(void* ctx, vh3_Notification a);
    friend hevc_error_t VSSHSDKAPI cb_rec_send(void* ctx, const vh3_Picture* pic, vh3_RecPictureInfo info);
    friend hevc_error_t VSSHSDKAPI cb_ms_send(void* ctx, const vh3_MediaSample* ms, vh3_NalInfo info);
    friend void cb_settings_change(void* ctx, const char* msg);

    uint32_t modifierValue(const std::string& str);
    uint16_t presetValue(const std::string& str);
};

/* This structure can contain only pointers and simple types */
struct PluginContext {
    PluginCtrl* ctrl{nullptr};
    Encoder* encoder{nullptr};
};

#endif //__DEE_PLUGINS_HEVC_ENC_BEAMR_IMPL_H__
