/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2017-2019, Dolby Laboratories
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

#include "hevc_dec_common.h"
#include <stdexcept>
#include <cstring>

namespace dee {

HevcStream::HevcStream(uint64_t _maxSize) {
    maxSize = _maxSize;
    data = new uint8_t[maxSize];
    size = 0;
}

HevcStream::~HevcStream() {
    if (data) {
        delete[] data;
        data = nullptr;
        maxSize = 0;
        size = 0;
    }
}

void HevcStream::addBytes(uint8_t* bytes, uint64_t byteNum) {
    if (byteNum > maxSize - size)
        throw std::runtime_error("Stream buffer overflow.");

    memcpy(data + size, bytes, byteNum);
    size += byteNum;
}

void HevcStream::dropBytes(uint64_t byteNum) {
    if (byteNum > size)
        byteNum = size;
    memmove(data, data + byteNum, size - byteNum);
    size -= byteNum;
}

enum NALU_TYPE {
    /* VCL NAL units */
    /* non-IRAP slices */
    NALU_TYPE_TRAIL_N = 0,
    NALU_TYPE_TRAIL_R = 1,
    NALU_TYPE_TSA_N = 2,
    NALU_TYPE_TSA_R = 3,
    NALU_TYPE_STSA_N = 4,
    NALU_TYPE_STSA_R = 5,
    NALU_TYPE_RADL_N = 6,
    NALU_TYPE_RADL_R = 7,
    NALU_TYPE_RASL_N = 8,
    NALU_TYPE_RASL_R = 9,
    /* IRAP slices */
    NALU_TYPE_BLA_W_LP = 16,
    NALU_TYPE_BLA_W_RADL = 17,
    NALU_TYPE_BLA_N_LP = 18,
    NALU_TYPE_IDR_W_RADL = 19,
    NALU_TYPE_IDR_N_LP = 20,
    NALU_TYPE_CRA = 21,
    /* non-VCL NAL units */
    NALU_TYPE_VPS = 32,
    NALU_TYPE_SPS = 33,
    NALU_TYPE_PPS = 34,
    NALU_TYPE_AUD = 35,
    NALU_TYPE_EOS = 36,
    NALU_TYPE_EOB = 37,
    NALU_TYPE_FD = 38,
    NALU_TYPE_PREFIX_SEI = 39,
    NALU_TYPE_SUFFIX_SEI = 40,
};

#define IS_VCL_NALU(nalu_type) (nalu_type < NALU_TYPE_VPS)

/**
 * @brief Finds start of an access unit in given data as defined in ITU-T H.265 7.4.2.4.4.
 *        Found address is written to the output parameter start_ptr.
 *        If start of access unit was not found, start_ptr
 *        is set to point to the end of data (data + size).
 * @param data pointer to the data to look for access unit in
 * @param size size of the data
 * @param start_ptr pointer to the start of the access unit or (data + size) if not found
 * @return Offset of the first VCL NALU in the found acess unit
 */
static uint64_t findAUStart(uint8_t* data, uint64_t size, uint8_t** start_ptr) {
    uint64_t offset;
    uint32_t window;
    bool startFound;
    uint8_t* start;
    int naluType;
    int firstSegmentInPic;

    if (size < 4)
        return size;

    offset = 4;
    startFound = false;
    start = NULL;
    while (offset < size) {
        window = ((uint32_t)data[offset - 4] << 24) + ((uint32_t)data[offset - 3] << 16)
                 + ((uint32_t)data[offset - 2] << 8) + (uint32_t)data[offset - 1];
        while ((window & 0xffffff00) != 0x00000100 && offset < size) {
            window <<= 8;
            window += (uint32_t)data[offset];
            ++offset;
        }

        /* check if found start of access unit */
        naluType = (window >> 1) & 0x0000003f;
        firstSegmentInPic = 0;
        if (offset + 1 < size && IS_VCL_NALU(naluType)) {
            firstSegmentInPic = data[offset + 1] >> 7;
        }

        if (firstSegmentInPic == 1) {
            /* if no end of access unit was found before
             * mark start of this VCL NAL unit as the end
             */
            if (!startFound) {
                start = data + (offset - 4);
                startFound = 1;
            }

            if (start > data && *(start - 1) == 0) {
                --start;
            }

            if (offset > 0 && data[offset - 1] == 0) {
                --offset;
            }

            *start_ptr = start;
            return offset;
        }
        else if (IS_VCL_NALU(naluType)) {
            /* reset start_found flag, because next VCL NAL unit was found
             * that is not first slice segment in picture
             */
            startFound = 0;
        }
        else if (naluType == NALU_TYPE_AUD || naluType == NALU_TYPE_VPS || naluType == NALU_TYPE_SPS
                 || naluType == NALU_TYPE_PPS || naluType == NALU_TYPE_PREFIX_SEI || (naluType >= 41 && naluType <= 44)
                 || (naluType >= 48 && naluType <= 55)) {
            /* if no end of access unit was found before
             * mark start of this non-VCL NAL unit as the end
             */
            if (!startFound) {
                start = data + (offset - 4);
                startFound = true;
            }
        }
        else if (naluType == NALU_TYPE_FD) {
            /* skip filler data */
            while (++offset < size && data[offset] == 0xff)
                ;
        }

        ++offset;
    }

    *start_ptr = data + size;
    return size;
}

bool HevcStream::findAU(uint64_t& startOffset, uint64_t& endOffset) {
    uint64_t firstVclOffset;
    uint8_t* start = data + size;
    uint8_t* end = data + size;

    firstVclOffset = findAUStart(data, size, &start);

    if (start > data && start < (data + size)) {
        /* found only part of an access unit,
         * return its end and mark start as not found
         */
        end = start;
        start = data + size;
    }
    else if (firstVclOffset < size - 3) {
        /* find start of the next access unit, skipping bytes up to and including
         * start code of the first VCL NALU in the first one
         */
        uint64_t skip_bytes = firstVclOffset + 3;
        findAUStart(data + skip_bytes, size - skip_bytes, &end);
    }

    startOffset = (unsigned int)(start - data);
    endOffset = (unsigned int)(end - data);

    return (endOffset < size);
}

bool HevcStream::findNALU(uint64_t& startOffset, uint64_t& endOffset, bool flush) {
    uint64_t start = 0, stop = 0, nz = 0;
    for (size_t i = 0; i < size; i++) {
        if ((data[i] == 1) && (nz >= 2)) {
            start = i + 1;
            break;
        }
        if (data[i] == 0)
            nz++;
        else
            nz = 0;
    }

    nz = 0;
    for (uint64_t i = start; i < size; i++) {
        if ((data[i] == 1) && (nz >= 2)) {
            stop = i - nz;
            break;
        }
        if (data[i] == 0)
            nz++;
        else
            nz = 0;
    }

    if (flush)
        stop = size;

    if (stop == 0)
        return false;

    startOffset = start;
    endOffset = stop;

    return true;
}
} // namespace dee
