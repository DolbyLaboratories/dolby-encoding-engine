#ifndef _HEVC_DEC_BEAMR_UTILS_H_
#define _HEVC_DEC_BEAMR_UTILS_H_

#include "hevc_dec_beamr_utils.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <stdexcept>

int string2int(const std::string& key, const std::string& value, int minValue, int maxValue) {
    int ival = std::stoi(value);
    if (ival < minValue || ival > maxValue) {
        std::string err = "Invalid '" + key + "' value.";
        throw std::runtime_error(err);
    }
    return ival;
}

bool string2bool(const std::string& key, const std::string& value) {
    if (value != "true" && value != "false") {
        std::string err = "Invalid '" + key + "' value.";
        throw std::runtime_error(err);
    }
    return (value == "true");
}

HevcDecFrameRate string2framerate(const std::string& str) {
    HevcDecFrameRate fr;
    memset(&fr, 0, sizeof(fr));
    if ("23.976" == str) {
        fr.framePeriod = 24000;
        fr.timeScale = 1001;
    }
    else if ("24" == str) {
        fr.framePeriod = 24;
        fr.timeScale = 1;
    }
    else if ("25" == str) {
        fr.framePeriod = 25;
        fr.timeScale = 1;
    }
    else if ("29.97" == str) {
        fr.framePeriod = 30000;
        fr.timeScale = 1001;
    }
    else if ("30" == str) {
        fr.framePeriod = 30;
        fr.timeScale = 1;
    }
    else if ("48" == str) {
        fr.framePeriod = 48;
        fr.timeScale = 1;
    }
    else if ("50" == str) {
        fr.framePeriod = 50;
        fr.timeScale = 1;
    }
    else if ("59.94" == str) {
        fr.framePeriod = 60000;
        fr.timeScale = 1001;
    }
    else if ("60" == str) {
        fr.framePeriod = 60;
        fr.timeScale = 1;
    }

    return fr;
}

#endif // _HEVC_DEC_BEAMR_UTILS_H_
