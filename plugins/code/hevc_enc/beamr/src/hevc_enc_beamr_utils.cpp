#include "hevc_enc_beamr_utils.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

template <typename Out>
void split(const std::string& s, const std::string delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;

    std::string tempString = s;
    for (;;) {
        auto pos = tempString.find(delim);

        if (!tempString.substr(0, pos).empty())
            *(result++) = tempString.substr(0, pos);

        if (std::string::npos == pos)
            break;
        tempString = tempString.substr(pos + delim.size(), std::string::npos);
    }
}

std::vector<std::string> split(const std::string& s, const std::string& delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

int64_t string2int(const std::string& name, const std::string& value, int64_t minValue, int64_t maxValue) {
    int64_t ival = std::stoll(value);
    if (ival < minValue || ival > maxValue) {
        std::string err = "Invalid '" + name + "' value.";
        throw std::runtime_error(err);
    }
    return ival;
}

bool string2bool(const std::string& name, const std::string& value) {
    if (value != "true" && value != "false") {
        std::string err = "Invalid '" + name + "' value.";
        throw std::runtime_error(err);
    }
    return (value == "true");
}

int64_t parseInt(const std::string& name, const std::string& value, const PropertyInfo* schema, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (name == std::string(schema[i].name)) {
            int64_t minVal = std::numeric_limits<int64_t>::min();
            int64_t maxVal = std::numeric_limits<int64_t>::max();
            if (schema[i].values) {
                auto tok = split(std::string(schema[i].values), ":");
                minVal = std::stoi(tok[0]);
                if (tok.size() > 1)
                    maxVal = std::stoi(tok[1]);
            }
            return string2int(name, value, minVal, maxVal);
        }
    }
    throw std::runtime_error("Unknown property: " + name);
}

std::string parseString(const std::string& name, const std::string& value, const PropertyInfo* schema, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (name == std::string(schema[i].name)) {
            if (schema[i].values) {
                auto tok = split(std::string(schema[i].values), ":");
                if (std::find(tok.begin(), tok.end(), value) == tok.end()) {
                    std::string err = "Invalid '" + name + "' value.";
                    throw std::runtime_error(err);
                }
            }
            return value;
        }
    }
    throw std::runtime_error("Unknown property: " + name);
}

std::string parseStringList(const std::string& name,
                            const std::string& value,
                            const std::string& listDelim,
                            const std::string& enums) {
    auto tok = split(std::string(enums), ":");
    auto items = split(value, listDelim);
    for (auto& item : items) {
        if (std::find(tok.begin(), tok.end(), item) == tok.end()) {
            std::string err = "Invalid '" + name + "' value (" + item + ").";
            throw std::runtime_error(err);
        }
    }
    return value;
}

bool parseBool(const std::string& name, const std::string& value, const PropertyInfo* schema, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (name == std::string(schema[i].name)) {
            return string2bool(name, value);
        }
    }
    throw std::runtime_error("Unknown property: " + name);
}

FramePeriod string2FramePeriod(const std::string& s) {
    FramePeriod fp;
    memset(&fp, 0, sizeof(fp));
    if ("23.976" == s) {
        fp.timeScale = 24000;
        fp.numUnitsInTick = 1001;
    }
    else if ("24" == s) {
        fp.timeScale = 24;
        fp.numUnitsInTick = 1;
    }
    else if ("25" == s) {
        fp.timeScale = 25;
        fp.numUnitsInTick = 1;
    }
    else if ("29.97" == s) {
        fp.timeScale = 30000;
        fp.numUnitsInTick = 1001;
    }
    else if ("30" == s) {
        fp.timeScale = 30;
        fp.numUnitsInTick = 1;
    }
    else if ("48" == s) {
        fp.timeScale = 48;
        fp.numUnitsInTick = 1;
    }
    else if ("50" == s) {
        fp.timeScale = 50;
        fp.numUnitsInTick = 1;
    }
    else if ("59.94" == s) {
        fp.timeScale = 60000;
        fp.numUnitsInTick = 1001;
    }
    else if ("60" == s) {
        fp.timeScale = 60;
        fp.numUnitsInTick = 1;
    }
    else if ("119.88" == s) {
        fp.timeScale = 120000;
        fp.numUnitsInTick = 1001;
    }
    else if ("120" == s) {
        fp.timeScale = 120;
        fp.numUnitsInTick = 1;
    }
    return fp;
}

static std::map<std::string, int> color_primaries_map = {
    {"bt_709", 1}, {"unspecified", 2}, {"bt_601_625", 5}, {"bt_601_525", 6}, {"bt_2020", 9},
};

static std::map<std::string, int> transfer_characteristics_map = {
    {"bt_709", 1}, {"unspecified", 2}, {"bt_601_625", 4}, {"bt_601_525", 6}, {"smpte_st_2084", 16}, {"std_b67", 18},
};

static std::map<std::string, int> matrix_coefficients_map = {
    {"bt_709", 1}, {"unspecified", 2}, {"bt_601_625", 4}, {"bt_601_525", 6}, {"bt_2020", 9},
};

int color_primaries2int(const std::string& s) {
    return color_primaries_map[s];
}
int transfer_characteristics2int(const std::string& s) {
    return transfer_characteristics_map[s];
}
int matrix_coefficients2int(const std::string& s) {
    return matrix_coefficients_map[s];
}

#ifdef WIN32
static const std::string dirSeparator = "\\";
#else
static const std::string dirSeparator = "/";
#endif

std::string abspath(const std::string& path, const std::string& head) {
    bool isAbsolute = false;
    if (path.size()) {
#ifdef WIN32
        if (path.size() > 2) {
            char d = path[0];
            char c = path[1];
            char s = path[2];
            isAbsolute = (':' == c && dirSeparator[0] == s && ((d >= 'a' && d <= 'z') || (d >= 'A' && d <= 'Z')));
        }
#else
        isAbsolute = (dirSeparator[0] == path[0]);
#endif
    }

    std::string r = path;
    if (!isAbsolute)
        r = head + dirSeparator + path;
    return r;
}

void checkFileReadable(const std::string& path) {
    std::ifstream file(path, std::ifstream::binary);
    if (!file.is_open()) {
        std::string msg = "Could not open file \"" + path + "\" for reading";
        throw std::runtime_error(msg);
    }
}

void checkFileWritable(const std::string& path) {
    std::ofstream file(path, std::ofstream::binary);
    if (!file.is_open()) {
        std::string msg = "Could not open file \"" + path + "\" for writing";
        throw std::runtime_error(msg);
    }
}
