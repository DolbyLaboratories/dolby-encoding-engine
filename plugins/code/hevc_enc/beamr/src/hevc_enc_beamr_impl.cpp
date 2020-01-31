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

#include "hevc_enc_beamr_impl.h"
#include "hevc_enc_beamr_utils.h"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <exception>
#include <fstream>
#include <map>

void prologue(void* ctx, const char* func, const char* vaargs, const char* parent, unsigned int line) {
    Encoder* enc = static_cast<Encoder*>(ctx);
    if (enc->debugLevel > 1) {
        std::cerr << "(" << parent << "::" << line << ") ";
        std::cerr << "prologue: " << func << "(" << vaargs << ")" << std::endl;
    }
}

template <typename RetvalType>
void epilogue(void* ctx, const char* func, const char* vaargs, const char* parent, unsigned int line, RetvalType) {
    Encoder* enc = static_cast<Encoder*>(ctx);
    if (enc->debugLevel > 1) {
        std::cerr << "(" << parent << "::" << line << ") ";
        std::cerr << "epilogue: " << func << "(" << vaargs << ")" << std::endl;
    }
}

template <typename RetvalType>
void epilogueWithRetval(
    void* ctx, const char* func, const char* vaargs, const char* parent, unsigned int line, RetvalType retval) {
    Encoder* enc = static_cast<Encoder*>(ctx);
    if (enc->debugLevel > 1) {
        std::cerr << "(" << parent << "::" << line << ") ";
        std::cerr << "epilogue: " << func << "(" << vaargs << ") - returned " << retval << std::endl;
    }
}

template <>
void epilogue(void* ctx, const char* func, const char* vaargs, const char* parent, unsigned int line, double retval) {
    epilogueWithRetval(ctx, func, vaargs, parent, line, retval);
}

template <>
void epilogue(void* ctx, const char* func, const char* vaargs, const char* parent, unsigned int line, int retval) {
    epilogueWithRetval(ctx, func, vaargs, parent, line, retval);
}

template <>
void epilogue(void* ctx, const char* func, const char* vaargs, const char* parent, unsigned int line, size_t retval) {
    epilogueWithRetval(ctx, func, vaargs, parent, line, retval);
}

template <>
void epilogue(void* ctx, const char* func, const char* vaargs, const char* parent, unsigned int line, bool retval) {
    epilogueWithRetval(ctx, func, vaargs, parent, line, retval);
}

extern "C" {
hevc_error_t VSSHSDKAPI cb_rec_send(void* ctx, const vh3_Picture* pic, vh3_RecPictureInfo info) {
    Encoder* enc = static_cast<Encoder*>(ctx);
    FUNCTION_P_RETVALA(enc, retval, enc->recSend, pic, info);
    return retval;
}
hevc_error_t VSSHSDKAPI cb_ms_send(void* ctx, const vh3_MediaSample* ms, vh3_NalInfo info) {
    Encoder* enc = static_cast<Encoder*>(ctx);
    FUNCTION_P_RETVALA(enc, retval, enc->msSend, ms, info);
    return retval;
}
hevc_error_t VSSHSDKAPI cb_notify(void* ctx, vh3_Notification a) {
    Encoder* enc = static_cast<Encoder*>(ctx);
    FUNCTION_P_RETVALA(enc, retval, enc->notify, a);
    return retval;
}
void cb_settings_change(void* ctx, const char* msg) {
    Encoder* enc = static_cast<Encoder*>(ctx);
    FUNCTION_P(enc, enc->settingsChange, msg);
}
}

Encoder::Encoder()
    : hEnc(NULL)
    , cbTable({vh3_default_malloc, vh3_default_free, vh3_default_pic_alloc, vh3_default_pic_free, vh3_default_ms_alloc,
               vh3_default_ms_realloc, vh3_default_ms_free, cb_rec_send, cb_ms_send, cb_notify, this}) {
}

Encoder::~Encoder() {
    FUNCTIONV_T(close);
    if (outBuffer)
        delete[] outBuffer;
}

std::string Encoder::getVersion() {
    return version;
}

std::string Encoder::getMessage() {
    std::lock_guard<std::mutex> lck(dataLock);
    std::string msg;
    for (auto& x : pendingMsgs) {
        if (msg.size())
            msg += "\n";
        msg += x;
    }
    pendingMsgs.clear();
    return msg;
}

void Encoder::checkPendingErrors() {
    std::string errMsg;
    {
        std::lock_guard<std::mutex> lck(dataLock);
        if (pendingErrors.size()) {
            for (auto& x : pendingErrors) {
                if (errMsg.size())
                    errMsg += "\n";
                errMsg += x;
            }
        }
    }
    if (errMsg.size())
        error("%s", errMsg.c_str());
}

void Encoder::checkErrorCode(const hevc_error_t errCode) {
    if (errCode != HEVC_OK) {
        if (errCode > HEVC_OK)
            message("%s (%d)", hevc_error_text(errCode), (int)errCode);
        else
            error("%s (%d)", hevc_error_text(errCode), (int)errCode);
    }
}

static std::map<std::string, hevc_rate_control_types_e> multi_pass2int = {
    {"off", HEVC_RATE_CONTROL_VBR},          // VBR
    {"1st", HEVC_RATE_CONTROL_DUAL_PASS_0},  // dual pass - 1st pass
    {"nth", HEVC_RATE_CONTROL_DUAL_PASS_0},  // dual pass - 1st pass
    {"last", HEVC_RATE_CONTROL_DUAL_PASS_1}, // dual pass - 2nd pass
};

static std::map<std::string, hevc_presets_e> preset2int = {
    {"insanely_slow", HEVC_PRESET_INSANELY_SLOW},
    {"ultra_slow", HEVC_PRESET_ULTRA_SLOW},
    {"very_slow", HEVC_PRESET_VERY_SLOW},
    {"slower", HEVC_PRESET_SLOWER},
    {"slow", HEVC_PRESET_SLOW},
    {"medium", HEVC_PRESET_MEDIUM},
    {"medium_plus", HEVC_PRESET_MEDIUM_PLUS},
    {"fast", HEVC_PRESET_FAST},
    {"faster", HEVC_PRESET_FASTER},
    {"ultra_fast", HEVC_PRESET_ULTRA_FAST},
    {"insanely_fast", HEVC_PRESET_INSANELY_FAST},
    // Can be set, but not explicitly exposed in DEE interface
    {"broadcast", HEVC_PRESET_BROADCAST},
    {"vod", HEVC_PRESET_VOD},
    {"ultralow_bitrate", HEVC_PRESET_ULTRALOW_BITRATE},
    {"gpu1", HEVC_PRESET_GPU1},
    {"gpu2", HEVC_PRESET_GPU2},
};

uint16_t Encoder::presetValue(const std::string& str) {
    uint16_t value = 0;
    if (!preset2int.count(str))
        error("Unknown preset - %s", str.c_str());
    else
        value = (uint16_t)preset2int[str];
    return value;
}

static std::map<std::string, hevc_modifiers_e> modifier2int = {
    {"low_delay", HEVC_MOD_LOW_DELAY},
    {"tune_psnr", HEVC_MOD_TUNE_PSNR},
    {"realtime", HEVC_MOD_REALTIME},
    {"cinema", HEVC_MOD_CINEMA},
    {"blueray", HEVC_MOD_BLUERAY},
    {"hdr", HEVC_MOD_HDR},
    {"gpu", HEVC_MOD_GPU},
    {"hlg", HEVC_MOD_HLG},
    {"vmaf", HEVC_MOD_TUNE_VMAF},
    {"low_bitrate", HEVC_MOD_LOW_BITRATE}, 
};

uint32_t Encoder::modifierValue(const std::string& str) {
    uint32_t mask = 0;
    auto tokens = split(str, "+");
    for (auto& x : tokens) {
        if (!modifier2int.count(x)) {
            error("Unknown modifier - %s", x.c_str());
            continue;
        }
        mask |= modifier2int[x];
    }
    return mask;
}

void Encoder::init(PluginCtrl& ctrl) {
    hevc_error_t errCode = HEVC_OK;

    debugLevel = ctrl.debug_level;

    int major, minor, rev, build;
    FUNCTION_T_RETVAL(errCode, hevc_get_version, 0, &major, &minor, &rev, &build);
    version += " v." + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(rev) + "."
               + std::to_string(build);
    checkErrorCode(errCode);

    memset(&settings, 0, sizeof(settings));
    settings.size = sizeof(settings);
    FUNCTION_T_RETVAL(errCode, hevce_default_settings, &settings);
    checkErrorCode(errCode);

    hevce_init_settings_t initSettings;
    memset(&initSettings, 0, sizeof(initSettings));
    initSettings.size = sizeof(initSettings);
    if (ctrl.preset.size())
        initSettings.preset = presetValue(ctrl.preset);
    if (ctrl.modifier.size())
        initSettings.modifier = modifierValue(ctrl.modifier);
    initSettings.width = ctrl.width;
    initSettings.height = ctrl.height;

    FUNCTION_T_RETVAL(errCode, hevce_init_settings, &settings, &initSettings, 0);
    checkErrorCode(errCode);

    settings.stream[0].params.bit_depth_chroma = settings.input.bit_depth_chroma = ctrl.bit_depth;
    settings.stream[0].params.bit_depth_luma = settings.input.bit_depth_luma = ctrl.bit_depth;
    settings.input.colorspace = VH3_YUV_420;
    settings.input.chroma_bytes_per_pel = ctrl.bit_depth > 8 ? 2 : 1;
    settings.input.luma_bytes_per_pel = ctrl.bit_depth > 8 ? 2 : 1;
    settings.input.width = ctrl.width;
    settings.input.height = ctrl.height;

    auto fp = string2FramePeriod(ctrl.frame_rate);
    settings.gop.time_scale = fp.timeScale;
    settings.gop.num_units_in_tick = fp.numUnitsInTick;

    settings.stream[0].rc.type = (int8_t)multi_pass2int[ctrl.multi_pass];
    settings.stream[0].rc.kbps = (int32_t)ctrl.data_rate;

    settings.stream[0].rc.max_kbps = (int32_t)ctrl.max_vbv_data_rate;
    settings.stream[0].rc.vbv_size = (uint32_t)ctrl.vbv_buffer_size;

    settings.stream[0].vui.video_signal_type_present_flag = 1;
    settings.stream[0].vui.video_full_range_flag = ctrl.range == "full" ? 1 : 0;
    settings.stream[0].vui.colour_description_present_flag = 1;
    settings.stream[0].vui.colour_primaries = (uint8_t)color_primaries2int(ctrl.color_primaries);
    settings.stream[0].vui.transfer_characteristics =
        (uint8_t)transfer_characteristics2int(ctrl.transfer_characteristics);
    settings.stream[0].vui.matrix_coeffs = (uint8_t)matrix_coefficients2int(ctrl.matrix_coefficients);
    settings.stream[0].vui.chroma_loc_info_present_flag = 1;
    settings.stream[0].vui.chroma_sample_loc_type_top_field = (uint8_t)ctrl.chromaloc;
    settings.stream[0].vui.chroma_sample_loc_type_bottom_field = (uint8_t)ctrl.chromaloc;
    settings.stream[0].sei.mastering_display_colour_volume_flag =
        (ctrl.mastering_display_sei_x1 >= 0 && ctrl.mastering_display_sei_y1 >= 0 && ctrl.mastering_display_sei_x2 >= 0
         && ctrl.mastering_display_sei_y2 >= 0 && ctrl.mastering_display_sei_x3 >= 0
         && ctrl.mastering_display_sei_y3 >= 0 && ctrl.mastering_display_sei_wx >= 0
         && ctrl.mastering_display_sei_wy >= 0 && ctrl.mastering_display_sei_max_lum >= 0
         && ctrl.mastering_display_sei_min_lum >= 0);
    if (settings.stream[0].sei.mastering_display_colour_volume_flag) {
        settings.stream[0].sei.mastering_display_colour_volume.display_primaries_x[0] =
            (uint16_t)ctrl.mastering_display_sei_x1;
        settings.stream[0].sei.mastering_display_colour_volume.display_primaries_y[0] =
            (uint16_t)ctrl.mastering_display_sei_y1;
        settings.stream[0].sei.mastering_display_colour_volume.display_primaries_x[1] =
            (uint16_t)ctrl.mastering_display_sei_x2;
        settings.stream[0].sei.mastering_display_colour_volume.display_primaries_y[1] =
            (uint16_t)ctrl.mastering_display_sei_y2;
        settings.stream[0].sei.mastering_display_colour_volume.display_primaries_x[2] =
            (uint16_t)ctrl.mastering_display_sei_x3;
        settings.stream[0].sei.mastering_display_colour_volume.display_primaries_y[2] =
            (uint16_t)ctrl.mastering_display_sei_y3;
        settings.stream[0].sei.mastering_display_colour_volume.white_point_x = (uint16_t)ctrl.mastering_display_sei_wx;
        settings.stream[0].sei.mastering_display_colour_volume.white_point_y = (uint16_t)ctrl.mastering_display_sei_wy;
        settings.stream[0].sei.mastering_display_colour_volume.max_display_mastering_luminance =
            (uint32_t)ctrl.mastering_display_sei_max_lum;
        settings.stream[0].sei.mastering_display_colour_volume.min_display_mastering_luminance =
            (uint32_t)ctrl.mastering_display_sei_min_lum;
    }

    settings.stream[0].sei.content_light_level_info_flag =
        (ctrl.light_level_max_content >= 0 && ctrl.light_level_max_frame_average >= 0);
    if (settings.stream[0].sei.content_light_level_info_flag) {
        settings.stream[0].sei.content_light_level_info.max_content_light_level =
            (uint16_t)ctrl.light_level_max_content;
        settings.stream[0].sei.content_light_level_info.max_pic_average_light_level =
            (uint16_t)ctrl.light_level_max_frame_average;
    }

    settings.gop.intra_period = ctrl.gop_intra_period;
    settings.gop.idr_period = ctrl.gop_idr_period;
    settings.gop.min_intra_period = ctrl.gop_min_intra_period;
    settings.gop.minigop_size = ctrl.gop_minigop_size;
    settings.gop.min_minigop_size = ctrl.gop_min_minigop_size;
    settings.gop.max_refs = ctrl.gop_max_refs;
    settings.me.scene_change = ctrl.me_scene_change;

    settings.stream[0].params.pps_cb_qp_offset = ctrl.pps_cb_qp_offset;
    settings.stream[0].params.pps_cr_qp_offset = ctrl.pps_cr_qp_offset;

    settings.stream[0].sei.pic_timing_flag = 1;
    settings.stream[0].vui.aspect_ratio_info_present_flag = 1;
    settings.stream[0].vui.aspect_ratio_idc = 1; // 1:1 (square)
    settings.gop.flags |= HEVC_GOP_ADD_AUD | HEVC_GOP_SPS_FOR_EACH_IDR;
    settings.gop.flags &= ~HEVC_GOP_NO_IDR_ON_SCENE_CHANGE;
    settings.stream[0].rc.flags |= HEVC_RC_FORCE_HRD_INFO;
    settings.stream[0].modifier |= 0x20;

    if (ctrl.uhd_bd) {
        message("Forcing UHD-BD/Dolby Vision profile 7 params.");
        settings.stream[0].modifier |= 0x20;
        settings.stream[0].params.general_profile_idc = 2;
        settings.stream[0].params.general_level_idc = 153;
        settings.stream[0].params.general_tier_flag = 1;
        for (int i = 0; i < VH3_MAX_T_LAYERS; i++) {
            settings.stream[0].params.layers[i].profile_idc = 2;
            settings.stream[0].params.layers[i].level_idc = 153;
            settings.stream[0].params.layers[i].tier_flag = 1;
        }
    }

    std::list<std::string> str;
    for (auto& tag : ctrl.param) {
        auto params = split(tag, ":");
        for (auto& x : params) {
            auto kv = split(x, "=");
            if ("stats_file" == kv[0] || "dualpass-file" == kv[0]) {
                if (kv.size() > 1)
                    ctrl.stats_file = kv[1];
                continue;
            }
            str.push_back(x);
        }
    }

    if (str.size()) {
        std::vector<char*> argv;
        for (auto& x : str)
            argv.push_back((char*)x.c_str());
        argv.push_back(nullptr);

        FUNCTION_T_RETVAL(errCode, hevce_read_cmd_line, &settings, argv.data());
        checkErrorCode(errCode);
        str.clear();
    }

    if (ctrl.native_config_file.size()) {
        std::string path = abspath(ctrl.native_config_file, ctrl.config_path);
        checkFileReadable(path);
        FUNCTION_T_RETVAL(errCode, hevce_read_config_file, &settings, path.c_str());
        checkErrorCode(errCode);
    }

    dualPassFileName = ctrl.stats_file;

    check_settings_log_t log{cb_settings_change, this};
    FUNCTION_T_RETVAL(errCode, hevce_check_settings, &settings, &log);
    checkErrorCode(errCode);

    FUNCTION_T_RETVAL(errCode, hevce_write_config_file, &settings, ctrl.temp_file[0].c_str());
    checkErrorCode(errCode);

    FUNCTION_T_RETVAL(errCode, vh3_enc_open, &hEnc, &cbTable, &settings);
    checkErrorCode(errCode);
    opened = true;

    if (HEVC_RATE_CONTROL_DUAL_PASS_1 == settings.stream[0].rc.type && dualPassFileName.size()) {
        checkFileReadable(dualPassFileName);
        std::ifstream infile(dualPassFileName, std::ifstream::binary);
        dualPassBuffer.clear();
        char c;
        while (infile.get(c))
            dualPassBuffer.push_back(c);

        if (0 == dualPassBuffer.size())
            error("Stats (dual-pass) file is empty.");

        FUNCTION_T_RETVAL(errCode, vh3_enc_setDualPassData, hEnc, dualPassBuffer.size(),
                          (vh3_dualpass_data_t)dualPassBuffer.data(), 0);
        checkErrorCode(errCode);
    }

    if (ctrl.gop_structure_in_file.size()) {
        std::string path = abspath(ctrl.gop_structure_in_file, ctrl.config_path);
        checkFileReadable(path);
        gopStructureIn.open(path, std::ifstream::in);
        parseGopStructureFile();
    }

    if (ctrl.gop_structure_out_file.size()) {
        std::string path = abspath(ctrl.gop_structure_out_file, ctrl.config_path);
        checkFileWritable(path);
        gopStructureOut.open(path, std::ofstream::out);
    }
}

static std::map<HevcEncFrameType, int> frameType2sliceType = {
    {HEVC_ENC_FRAME_TYPE_AUTO, (int)VH3_SLICE_AUTO}, {HEVC_ENC_FRAME_TYPE_IDR, (int)VH3_SLICE_I},
    {HEVC_ENC_FRAME_TYPE_I, (int)VH3_SLICE_I},       {HEVC_ENC_FRAME_TYPE_P, (int)VH3_SLICE_P},
    {HEVC_ENC_FRAME_TYPE_B, (int)VH3_SLICE_B},       {HEVC_ENC_FRAME_TYPE_BREF, (int)VH3_SLICE_B}};

void Encoder::feed(const HevcEncPicture* in) {
    checkPendingErrors();
    if (in) {
        vh3_Picture* pic;
        FUNCTION_T_RETVAL(pic, cbTable.pic_alloc, cbTable.app_context, settings.input.width, settings.input.height,
                          settings.input.luma_bytes_per_pel, settings.input.chroma_bytes_per_pel,
                          settings.input.bit_depth_luma, settings.input.bit_depth_chroma, VH3_YUV_420);
        if (pic == nullptr)
            error("pic is nullptr");

        vh3_SrcPictureInfo picInfo;
        hevc_error_t errCode;
        FUNCTION_T_RETVAL(errCode, vh3_enc_initPictureInfo, &picInfo);
        checkErrorCode(errCode);

        picInfo.mediatime = frameIndex;
        pic->timestamp = frameIndex;

        picInfo.mod.slice_type = (int8_t)frameType2sliceType[in->frameType];
        if (HEVC_ENC_FRAME_TYPE_IDR == in->frameType)
            picInfo.mod.idr_flag = 1;

        while (slices.size() && slices.front().index < (int64_t)frameIndex) {
            message("pop SliceInfo: %lld", slices.front().index);
            slices.pop_front();
        }

        if (slices.size() && slices.front().index == (int64_t)frameIndex) {
            message("Force slice: %llu, %d, %d", frameIndex, (int)slices.front().type, (int)slices.front().idr_flag);
            picInfo.mod.slice_type = (int8_t)slices.front().type;
            picInfo.mod.idr_flag = slices.front().idr_flag;
            if (slices.size() == 1) {
                picInfo.mod.slice_type = (int8_t)VH3_SLICE_P;
                picInfo.mod.idr_flag = 0;
            }
        }

        FUNCTION_T(readFrame, in, pic);

        FUNCTION_T_RETVAL(errCode, vh3_enc_waitForCanSet, hEnc);
        checkErrorCode(errCode);

        FUNCTION_T_RETVAL(errCode, vh3_enc_setPicture, hEnc, pic, &picInfo);
        checkErrorCode(errCode);

        if (settings.mt.disable) {
            FUNCTION_T_RETVAL(errCode, vh3_encode, hEnc, nullptr);
            checkErrorCode(errCode);
        }

        if (debugLevel > 0) {
            std::lock_guard<std::mutex> lck(dataLock);
            message("Fed frame[" + std::to_string(frameIndex) + "]");
        }
        frameIndex++;
    }
}

void Encoder::flush() {
    checkPendingErrors();
    if (!flushing) {
        flushing = true;
        hevc_error_t errCode;
        FUNCTION_T_RETVAL(errCode, vh3_enc_flush, hEnc);
        checkErrorCode(errCode);

        if (settings.mt.disable) {
            FUNCTION_T_RETVAL(errCode, vh3_encode, hEnc, nullptr);
            checkErrorCode(errCode);
        }

        FUNCTION_T_RETVAL(errCode, vh3_enc_waitForEncode, hEnc);
        checkErrorCode(errCode);
    }
}

void Encoder::close() {
    if (opened) {
        std::lock_guard<std::mutex> lck(dataLock);
        FUNCTIONV_T(releaseNalUnits);
        hevc_error_t errCode;
        if (HEVC_RATE_CONTROL_DUAL_PASS_0 == settings.stream[0].rc.type && dualPassFileName.size()) {
            size_t sz;
            vh3_dualpass_data_t dpData;
            FUNCTION_T_RETVAL(errCode, vh3_enc_getDualPassData, hEnc, &sz, &dpData);
            checkErrorCode(errCode);

            std::ofstream outfile(dualPassFileName, std::ofstream::binary);
            if (!outfile.is_open())
                error("Could not open stats (dual-pass) file for writing.");

            outfile.write((char*)dpData, sz);
            outfile.flush();
            outfile.close();
        }

        FUNCTION_T_RETVAL(errCode, vh3_enc_purge, hEnc);
        checkErrorCode(errCode);
        FUNCTION_T_RETVAL(errCode, vh3_enc_close, hEnc);
        checkErrorCode(errCode);
        opened = false;
    }

    if (gopStructureIn.is_open())
        gopStructureIn.close();

    if (gopStructureOut.is_open()) {
        reportSliceInfo(nullptr);
        gopStructureOut.close();
    }
}

void Encoder::readFrame(const HevcEncPicture* in, vh3_Picture* pic) {
    uint8_t* picData{nullptr};
    uint8_t* inData{nullptr};
    uint32_t compStride{0};
    uint16_t compWidth{0};
    uint16_t compHeight{0};

    for (uint8_t comp{0}; comp < 3; comp++) {
        if (comp == 0) {
            compStride = pic->stride[comp] * pic->luma_bytes_per_pel;
            compWidth = pic->width * pic->luma_bytes_per_pel;
            compHeight = pic->height;
        }
        else {
            compStride = pic->stride[comp] * pic->chroma_bytes_per_pel;
            compWidth = (pic->width >> 1) * pic->chroma_bytes_per_pel;
            compHeight = (pic->height >> 1);
        }

        picData = static_cast<uint8_t*>(pic->data[comp]);
        inData = static_cast<uint8_t*>(in->plane[comp]);

        for (uint16_t i = 0; i < compHeight; i++) {
            memcpy(picData, inData, compWidth);
            picData += compStride;
            inData += compWidth;
        }
    }
}

void Encoder::releaseNalUnits() {
    while (nalUnitsToRelease) {
        hevc_error_t errCode;
        FUNCTION_T_RETVAL(errCode, vh3_enc_releaseMediaSample, hEnc, qMS.front().ms);
        checkErrorCode(errCode);
        qMS.pop_front();
        nalUnitsToRelease--;
    }
}

void Encoder::getNal(HevcEncOutput* out, uint64_t maxSize) {
    out->nalNum = 0;
    checkPendingErrors();
    std::lock_guard<std::mutex> lck(dataLock);
    FUNCTIONV_T(releaseNalUnits);
    
    if (qMS.empty())
        return;

    if (maxSize > outBufferSize) {
        if (outBuffer)
            delete[] outBuffer;
        outBuffer = new uint8_t[maxSize];
        outBufferSize = maxSize;
    }

    uint8_t* curBufPos = outBuffer;
    uint64_t remainingBytes = maxSize;
    size_t nalIdx = 0;
    nal.clear();
    while (remainingBytes && nalIdx < qMS.size()) {
        vh3_Nal u = qMS[nalIdx++];
        auto ms = u.ms;
        if ((uint64_t)ms->used_size < remainingBytes) {
            HevcEncNal tmp;
            if (debugLevel > 0)
                message("ms->used_size: " + std::to_string(ms->used_size) + ", remainingBytes: " + std::to_string(remainingBytes));
            FUNCTION_T(memcpy, curBufPos, ms->data, ms->used_size);
            tmp.payload = curBufPos;
            tmp.size = ms->used_size;
            tmp.type = (HevcEncNalType)u.info.type;
            curBufPos += ms->used_size;
            nal.push_back(tmp);
            nalUnitsToRelease++;
            remainingBytes -= (uint64_t)ms->used_size;
        }
        else {
            break;
        }
    }

    out->nal = nal.data();
    out->nalNum = nal.size();
}

void Encoder::parseGopStructureFile() {
    uint64_t lineNum = 0;
    std::string line;
    std::getline(gopStructureIn, line); // header
    lineNum++;
    while (std::getline(gopStructureIn, line)) {
        lineNum++;
        auto args = split(line, " ");

        if (args.size() > 1) {
            SliceInfo s;
            s.index = std::stoll(args[0]);
            s.type = (vh3_SliceType)std::stoul(args[1]);
            if (args.size() > 2)
                s.idr_flag = (int8_t)std::stoul(args[2]);

            if (s.index < 0) {
                message("gop_structure_in_file: Ignoring line %llu. Invalid 'index' value.", lineNum);
                continue;
            }

            if (s.type != VH3_SLICE_B && s.type != VH3_SLICE_P && s.type != VH3_SLICE_I && s.type != -1) {
                message("gop_structure_in_file: Ignoring line %llu. Invalid 'type' value.", lineNum);
                continue;
            }

            slices.push_back(s);
        }
    }

    auto comparator = [](const SliceInfo& a, const SliceInfo& b) { return a.index < b.index; };
    std::sort(slices.begin(), slices.end(), comparator);
}

hevc_error_t Encoder::notify(vh3_Notification a) {
    if (VH3_MSG == a.type) {
        if ((int)a.data.msg.code < 0) {
            std::lock_guard<std::mutex> lck(dataLock);
            snprintf(stringBuffer, Encoder::MAX_STRING_SIZE, "%s (%d)", a.data.msg.text, (int)a.data.msg.code);
            pendingErrors.push_back(std::string(stringBuffer));
        }
        else if (debugLevel > 0) {
            std::lock_guard<std::mutex> lck(dataLock);
            snprintf(stringBuffer, Encoder::MAX_STRING_SIZE, "%s (%d)", a.data.msg.text, (int)a.data.msg.code);
            message(std::string(stringBuffer));
        }
    }
    return HEVC_OK;
}

void Encoder::reportSliceInfo(vh3_RecPictureInfo* info) {
    if (gopStructureOut.is_open()) {
        if (0 == recFrameIndex && gopSlices.empty())
            gopStructureOut << "index type idr_flag" << std::endl;

        bool newGop = false;
        SliceInfo s;
        if (info) {
            s.index = info->stat.poc;
            s.type = info->stat.type;
            s.idr_flag = info->stat.idr_flag;
            for (auto& x : gopSlices) {
                if (x.index == s.index) {
                    newGop = true;
                    break;
                }
            }
        }

        if (!newGop && info)
            gopSlices.push_back(s);

        if (newGop || !info) {
            auto comparator = [](const SliceInfo& a, const SliceInfo& b) { return a.index < b.index; };
            std::sort(gopSlices.begin(), gopSlices.end(), comparator);

            for (auto& x : gopSlices) {
                gopStructureOut << recFrameIndex++;
                gopStructureOut << " " << (int)x.type;
                gopStructureOut << " " << (int)x.idr_flag;
                gopStructureOut << std::endl;
            }

            gopSlices.clear();

            if (info)
                gopSlices.push_back(s);
        }
    }
}

hevc_error_t Encoder::recSend(const vh3_Picture* pic, vh3_RecPictureInfo info) {
    reportSliceInfo(&info);
    hevc_error_t errCode;
    FUNCTION_T_RETVAL(errCode, vh3_enc_releasePicture, hEnc, pic);
    checkErrorCode(errCode);
    return HEVC_OK;
}

hevc_error_t Encoder::msSend(const vh3_MediaSample* ms, vh3_NalInfo info) {
    std::lock_guard<std::mutex> lck(dataLock);
    vh3_Nal u{info, ms};
    qMS.emplace_back(u);
    return HEVC_OK;
}

void Encoder::settingsChange(const char* msg) {
    message(std::string(msg));
}

void Encoder::message(const char* fmt, ...) {
    if (debugLevel > 0) {
        std::lock_guard<std::mutex> lck(dataLock);
        va_list args;
        va_start(args, fmt);
        memset(stringBuffer, 0, Encoder::MAX_STRING_SIZE);
        vsnprintf(stringBuffer, Encoder::MAX_STRING_SIZE - 1, fmt, args);
        va_end(args);
        pendingMsgs.push_back(std::string(stringBuffer));
    }
}

void Encoder::message(const std::string& s) {
    if (debugLevel > 0) {
        pendingMsgs.push_back(s);
    }
}

void Encoder::error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    memset(stringBuffer, 0, Encoder::MAX_STRING_SIZE);
    vsnprintf(stringBuffer, Encoder::MAX_STRING_SIZE - 1, fmt, args);
    va_end(args);
    throw std::runtime_error(stringBuffer);
}
