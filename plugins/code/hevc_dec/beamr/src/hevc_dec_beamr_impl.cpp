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

#include "hevc_dec_beamr_impl.h"
#include "hevc_dec_beamr_utils.h"
#include <cstdarg>
#include <cstdio>
#include <exception>

void prologue(void* ctx, const char* func, const char* vaargs, const char* parent, unsigned int line) {
    Decoder* dec = static_cast<Decoder*>(ctx);
    if (dec->debugLevel > 1) {
        std::cerr << "(" << parent << "::" << line << ") ";
        std::cerr << "prologue: " << func << "(" << vaargs << ")" << std::endl;
    }
}

template <typename RetvalType>
void epilogue(void* ctx, const char* func, const char* vaargs, const char* parent, unsigned int line, RetvalType) {
    Decoder* dec = static_cast<Decoder*>(ctx);
    if (dec->debugLevel > 1) {
        std::cerr << "(" << parent << "::" << line << ") ";
        std::cerr << "epilogue: " << func << "(" << vaargs << ")" << std::endl;
    }
}

template <typename RetvalType>
void epilogueWithRetval(
    void* ctx, const char* func, const char* vaargs, const char* parent, unsigned int line, RetvalType retval) {
    Decoder* dec = static_cast<Decoder*>(ctx);
    if (dec->debugLevel > 1) {
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
    Decoder* dec = static_cast<Decoder*>(ctx);
    FUNCTION_P_RETVALA(dec, retval, dec->recSend, pic, info);
    return retval;
}
hevc_error_t VSSHSDKAPI cb_ms_send(void* ctx, const vh3_MediaSample* pMs, vh3_NalInfo info) {
    Decoder* dec = static_cast<Decoder*>(ctx);
    FUNCTION_P_RETVALA(dec, retval, dec->msSend, pMs, info);
    return retval;
}
hevc_error_t VSSHSDKAPI cb_notify(void* ctx, vh3_Notification a) {
    Decoder* dec = static_cast<Decoder*>(ctx);
    FUNCTION_P_RETVALA(dec, retval, dec->notify, a);
    return retval;
}
}

Decoder::Decoder()
    : stream(dee::HevcStream(MAX_NALU_SIZE))
    , hDec(NULL)
    , cbTable({vh3_default_malloc, vh3_default_free, vh3_default_pic_alloc, vh3_default_pic_free, vh3_default_ms_alloc,
               vh3_default_ms_realloc, vh3_default_ms_free, cb_rec_send, cb_ms_send, cb_notify, this}) {
}

Decoder::~Decoder() {
    FUNCTIONV_T(close);
    if (outBuffer)
        delete[] outBuffer;
}

std::string Decoder::getVersion() {
    return version;
}

std::string Decoder::getMessage() {
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

void Decoder::checkPendingErrors() {
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

void Decoder::checkErrorCode(const hevc_error_t errCode) {
    if (errCode != HEVC_OK) {
        if (errCode > HEVC_OK)
            message("%s (%d)", hevc_error_text(errCode), (int)errCode);
        else
            error("%s (%d)", hevc_error_text(errCode), (int)errCode);
    }
}

void Decoder::init(PluginCtrl& ctrl) {
    hevc_error_t errCode = HEVC_OK;

    debugLevel = ctrl.debug_level;
    int nthreads = ctrl.mt_num_threads ? ctrl.mt_num_threads : (int)std::thread::hardware_concurrency();

    // Cross-assign to convert frame-rate into frame-period.
    vuiNumUnitsInTick = ctrl.frame_rate.timeScale;
    vuiTimeScale = ctrl.frame_rate.framePeriod;
    useSpsFramerate = ctrl.use_sps_frame_rate;

    int major, minor, rev, build;
    errCode = hevc_get_version(0, &major, &minor, &rev, &build);
    version += " v." + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(rev) + "."
               + std::to_string(build);
    checkErrorCode(errCode);

    memset(&settings, 0, sizeof(settings));
    settings.size = sizeof(settings);
    errCode = hevcd_default_settings(&settings);
    checkErrorCode(errCode);

    settings.mt.disable = ctrl.mt_disable ? 1 : 0;
    settings.mt.num_threads = (uint8_t)nthreads;
    settings.mt.num_wf_lines = (uint8_t)ctrl.mt_num_wf_lines;
    settings.mt.num_frames = (uint8_t)ctrl.mt_num_frames;
    settings.mt.num_input_units = (uint8_t)ctrl.mt_num_input_units;
    settings.output_delay = (int32_t)ctrl.output_delay;

    // Bitmasks
    settings.flags = ctrl.flags;
    settings.mt.flags = ctrl.mt_flags;
    settings.mt.aff_masks[0] = ctrl.mt_aff_mask;
    settings.disable_cpu_extensions = ctrl.disable_cpu_extensions;

    errCode = hevcd_check_settings(&settings);
    checkErrorCode(errCode);
    
    errCode = vh3_dec_open(&hDec, &settings, &cbTable);
    checkErrorCode(errCode);
    opened = true;
}

void Decoder::feed(uint8_t* bytes, uint64_t bytesNum) {
    checkPendingErrors();
    if (bytes && bytesNum)
        FUNCTION_T(stream.addBytes, bytes, bytesNum);
    bool consumed = false;
    do {
        FUNCTIONV_T_RETVAL(consumed, consumeStream);
    } while (consumed);
}

void Decoder::flush() {
    checkPendingErrors();
    if (!flushed) {
        FUNCTION_T(feed, nullptr, 0);
        flushing = true;
        FUNCTION_T(feed, nullptr, 0); // Feed all remaining stream data (the last NALU).

        std::thread thread = std::thread(&Decoder::flushDecodeThread, this);

        hevc_error_t errCode;
        FUNCTION_T_RETVAL(errCode, vh3_dec_flush, hDec);
        checkErrorCode(errCode);

        FUNCTION_T_RETVAL(errCode, vh3_dec_waitForDecode, hDec);
        checkErrorCode(errCode);

        flushing = false;
        flushed = true;
        if (thread.joinable())
            thread.join();
    }
}

void Decoder::close() {
    if (opened) {
        std::lock_guard<std::mutex> lck(dataLock);
        hevc_error_t errCode;
        FUNCTION_T_RETVAL(errCode, vh3_dec_purge, hDec);
        checkErrorCode(errCode);
        FUNCTION_T_RETVAL(errCode, vh3_dec_close, hDec);
        checkErrorCode(errCode);
        opened = false;
    }
}

bool Decoder::consumeStream() {
    uint64_t startOffset, endOffset;
    bool found;
    FUNCTION_T_RETVAL(found, stream.findNALU, startOffset, endOffset, flushing);
    if (found) {
        FUNCTION_T(sendToDecode, startOffset, endOffset);
    }
    return found;
}

void Decoder::sendToDecode(uint64_t startOffset, uint64_t endOffset) {
    vh3_MediaSample* mediaSample;
    FUNCTION_T_RETVAL(mediaSample, cbTable.ms_alloc, nullptr, endOffset - startOffset);
    FUNCTION_T(memcpy, mediaSample->data, stream.data + startOffset, endOffset - startOffset);
    mediaSample->used_size = endOffset - startOffset;

    FUNCTION_T(stream.dropBytes, endOffset);

    hevc_error_t errCode;
    FUNCTION_T_RETVAL(errCode, vh3_dec_waitForFeed, hDec);
    checkErrorCode(errCode);

    FUNCTION_T_RETVAL(errCode, vh3_dec_feedData, hDec, mediaSample, 0);
    checkErrorCode(errCode);

    FUNCTION_T_RETVAL(errCode, vh3_decode, hDec, nullptr);
    checkErrorCode(errCode);
}

void Decoder::cropFrame(vh3_Picture* dst, const vh3_Picture* const src, const vh3_PictureDisplay& params) {
    memcpy(dst, src, sizeof(vh3_Picture));

    if (params.crop_bot > 0 || params.crop_top > 0 || params.crop_left > 0 || params.crop_right > 0) {
        uint8_t** data = (uint8_t**)dst->data;

        data[0] += (params.crop_left + (params.crop_top * dst->stride[0])) * dst->luma_bytes_per_pel;
        data[1] += (params.crop_left + (params.crop_top * dst->stride[1])) * dst->chroma_bytes_per_pel / 2;
        data[2] += (params.crop_left + (params.crop_top * dst->stride[2])) * dst->chroma_bytes_per_pel / 2;
        dst->height -= params.crop_top + params.crop_bot;
        dst->width -= params.crop_left + params.crop_right;
    }
}

void Decoder::writeFrame(const vh3_Picture* pPic, HevcDecPicture& outPic) {
    uint8_t* ctrl{nullptr};
    uint32_t compStride{0};
    uint16_t compWidth{0};
    uint16_t compHeight{0};

    uint64_t maxSize = pPic->width * pPic->width * pPic->luma_bytes_per_pel * 3;
    if (maxSize > outBufferSize) {
        if (outBuffer)
            delete[] outBuffer;
        outBuffer = new uint8_t[maxSize];
        outBufferSize = maxSize;
    }

    uint8_t* buffPos = outBuffer;

    for (uint8_t comp = 0; comp < 3; comp++) {
        if (0 == comp) {
            compStride = pPic->stride[comp] * pPic->luma_bytes_per_pel;
            compWidth = pPic->width * pPic->luma_bytes_per_pel;
            compHeight = pPic->height;
        }
        else {
            compStride = pPic->stride[comp] * pPic->chroma_bytes_per_pel;
            compWidth = (pPic->width >> 1) * pPic->chroma_bytes_per_pel;
            compHeight = pPic->chromaFmt == VH3_YUV_422 ? pPic->height :
                                                          pPic->chromaFmt == VH3_YUV_420 ? (pPic->height >> 1) : 0;
        }

        ctrl = static_cast<uint8_t*>(pPic->data[comp]);

        outPic.plane[comp] = buffPos;
        outPic.stride[comp] = (int)compStride;

        if (0 == comp)
            outPic.lumaSize = compHeight * compWidth;
        else
            outPic.chromaSize = compHeight * compWidth;

        for (uint16_t i = 0; i < compHeight; i++) {
            memcpy(buffPos, ctrl, compWidth);
            ctrl += compStride;
            buffPos += compWidth;
        }
    }

    outPic.width = pPic->width;
    outPic.height = pPic->height;
}

static HevcDecColorSpace chromaFormat2colorSpace(const vh3_ChromaFmt& chromaFormat) {
    switch (chromaFormat) {
    case VH3_YUV_400:
        return HEVC_DEC_COLOR_SPACE_I400;
    case VH3_YUV_420:
        return HEVC_DEC_COLOR_SPACE_I420;
    case VH3_YUV_422:
        return HEVC_DEC_COLOR_SPACE_I422;
    case VH3_YUV_444:
        return HEVC_DEC_COLOR_SPACE_I444;
    default:
        return HEVC_DEC_COLOR_SPACE_I444;
    }
}

static HevcDecFrameType slcieType2frameType(const vh3_SliceType& sliceType) {
    switch (sliceType) {
    case VH3_SLICE_B:
        return HEVC_DEC_FRAME_TYPE_B;
    case VH3_SLICE_P:
        return HEVC_DEC_FRAME_TYPE_P;
    case VH3_SLICE_I:
        return HEVC_DEC_FRAME_TYPE_I;
    case VH3_SLICE_UNKNOWN:
    default:
        return HEVC_DEC_FRAME_TYPE_AUTO;
    }
}

bool Decoder::getPic(HevcDecPicture& outPic) {
    checkPendingErrors();

    std::lock_guard<std::mutex> lck(dataLock);
    if (qRec.empty())
        return false;

    OutPictureData picData = qRec.front();
    qRec.pop();

    vh3_Picture cropPic;
    memset(&cropPic, 0, sizeof(cropPic));
    cropFrame(&cropPic, picData.pic, picData.picInfo.display);
    writeFrame(&cropPic, outPic);

    const vh3_Sps* pSps;
    hevc_error_t errCode;
    FUNCTION_T_RETVAL(errCode, vh3_dec_getSps, hDec, picData.pic, &pSps);
    checkErrorCode(errCode);

    // Cross-assign to convert frame-period into frame-rate.
    if (useSpsFramerate && pSps->vui_parameters_present_flag) {
        outPic.frameRate.framePeriod = pSps->vui_parameters.vui_time_scale;
        outPic.frameRate.timeScale = pSps->vui_parameters.vui_num_units_in_tick;
    }
    else {
        outPic.frameRate.framePeriod = vuiTimeScale;
        outPic.frameRate.timeScale = vuiNumUnitsInTick;
    }

    outPic.bitDepth = picData.pic->luma_bits;
    outPic.colorSpace = chromaFormat2colorSpace(picData.pic->chromaFmt);
    outPic.frameType = slcieType2frameType(picData.picInfo.stat.type);
    outPic.transferCharacteristics = 0;
    outPic.matrixCoeffs = 0;

    if (pSps->vui_parameters_present_flag) {
        outPic.transferCharacteristics = (int)pSps->vui_parameters.transfer_characteristics;
        outPic.matrixCoeffs = (int)pSps->vui_parameters.matrix_coeffs;
    }

    FUNCTION_T_RETVAL(errCode, vh3_dec_releasePicture, hDec, picData.pic);
    checkErrorCode(errCode);
    return true;
}

void Decoder::flushDecodeThread() {
    while(flushing) {
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
        FUNCTION_T(vh3_decode, hDec, nullptr);
    }
}

hevc_error_t Decoder::notify(vh3_Notification a) {
    if (VH3_MSG == a.type) {
        if ((int)a.data.msg.code < 0) {
            std::lock_guard<std::mutex> lck(dataLock);
            snprintf(stringBuffer, Decoder::MAX_STRING_SIZE, "%s (%d)", a.data.msg.text, (int)a.data.msg.code);
            pendingErrors.push_back(std::string(stringBuffer));
        }
        else if (debugLevel > 0) {
            std::lock_guard<std::mutex> lck(dataLock);
            snprintf(stringBuffer, Decoder::MAX_STRING_SIZE, "%s (%d)", a.data.msg.text, (int)a.data.msg.code);
            pendingMsgs.push_back(std::string(stringBuffer));
        }
    }
    return HEVC_OK;
}

hevc_error_t Decoder::recSend(const vh3_Picture* pPic, vh3_RecPictureInfo info) {
    std::lock_guard<std::mutex> lck(dataLock);
    qRec.push({pPic, info});
    return HEVC_OK;
}

hevc_error_t Decoder::msSend(const vh3_MediaSample* pMs, vh3_NalInfo) {
    hevc_error_t errCode;
    FUNCTION_T_RETVAL(errCode, vh3_dec_releaseMediaSample, hDec, pMs);
    checkErrorCode(errCode);
    return errCode;
}

void Decoder::message(const char* fmt, ...) {
    std::lock_guard<std::mutex> lck(dataLock);
    va_list args;
    va_start(args, fmt);
    memset(stringBuffer, 0, Decoder::MAX_STRING_SIZE);
    vsnprintf(stringBuffer, Decoder::MAX_STRING_SIZE - 1, fmt, args);
    va_end(args);
    pendingMsgs.push_back(std::string(stringBuffer));
}

void Decoder::error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    memset(stringBuffer, 0, Decoder::MAX_STRING_SIZE);
    vsnprintf(stringBuffer, Decoder::MAX_STRING_SIZE - 1, fmt, args);
    va_end(args);
    throw std::runtime_error(stringBuffer);
}
