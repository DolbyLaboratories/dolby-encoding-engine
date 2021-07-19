/*
* BSD 3-Clause License
*
* Copyright (c) 2018-2019, Dolby Laboratories
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

#include "ProresDecPlugin.h"
#include "ProResDecoder.h"
#include <stdint.h>

class ProresDecApple : public ProresDecPlugin
{
public:
    ProresDecApple();

    Status    init(const ProresDecInitParams* initParams);
    Status    process(const ProresDecInput* in, ProresDecOutput* out);
    Status    close();
    Status    setProperty(const Property* property);
    Status    getProperty(Property* property); 
    void      setPRPixelFormat();
    void      makePixelBuffer();
    bool      convertDecodedPicture();

    static void             fillProperties();
    static PropertyInfo*    getPropTable();
    static int              getPropTablesize();

private:
    
    static std::vector<PropertyInfo> propVector;

    int             width;
    int             height;
    int             threads;
    int             bytesPerRow;
    int             bufferSize;
    int             decImgSize;
    PRPixelFormat   decoderFormat;
    PRDecoderRef    decoder;
    PRPixelBuffer   pixelBuffer;
    unsigned char*  outBuffer;
    uint8_t*        scratchMem;
    ProresDecFormat pluginFormat;
};

std::vector<PropertyInfo> ProresDecApple::propVector;

bool ProresDecApple::convertDecodedPicture()
{
    if (decoderFormat == kPRFormat_b64a)
    {
        if (bufferSize % 8)
        {
            msg = "assert(bufferSize % 8), bufferSize=" + std::to_string(bufferSize);
            return false;
        }

        int bytesToSkipPerRow = bytesPerRow - (width*8);
        int planeBytes = width*height*2;

        uint8_t *r = scratchMem;
        uint8_t *g = r + planeBytes;
        uint8_t *b = g + planeBytes;
        uint8_t *d = (uint8_t*)outBuffer;

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                // switching bytes from big endian to little endian, ommiting bytes 0 and 1 (alpha channel) 
                *r++ = d[3];
                *r++ = d[2];
                *g++ = d[5];
                *g++ = d[4];
                *b++ = d[7];
                *b++ = d[6];
                d += 8;
            }
            d += bytesToSkipPerRow;
        }

        bufferSize = width*height*6;
        memcpy(outBuffer, scratchMem, bufferSize);
        return true;
    }
    else if (decoderFormat == kPRFormat_v216)
    {
        if (bufferSize % 8)
        {
            msg = "assert(bufferSize % 8), bufferSize=" + std::to_string(bufferSize);
            return false;
        }

        int wordsToSkipPerRow = (bytesPerRow - (width*4)) / sizeof(uint16_t);
        int lumaPixels = width*height;
        int chromaPixels = height*width/2;

        uint16_t *y = (uint16_t*)scratchMem;
        uint16_t *u = y + lumaPixels;
        uint16_t *v = u + chromaPixels;
        uint16_t *d = (uint16_t*)outBuffer;

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width/2; j++) {
                *u++ = *d++;
                *y++ = *d++;
                *v++ = *d++;
                *y++ = *d++;
            }
            d += wordsToSkipPerRow;
        }

        bufferSize = width*height*4;
        memcpy(outBuffer, scratchMem, bufferSize);
        return true;
    }
    else
    {
        return false;
    }
}

void ProresDecApple::setPRPixelFormat()
{
    switch (pluginFormat)
    {
    case RGB48LE:
        decoderFormat = kPRFormat_b64a;
        break;
    case YUV422P16LE:
        decoderFormat = kPRFormat_v216;
    default:
        break;
    }
};

void ProresDecApple::makePixelBuffer()
{
    pixelBuffer = {
        outBuffer,
        bytesPerRow,
        decoderFormat,
        width,
        height
    };
}

ProresDecApple::ProresDecApple()
    :width(0)
    ,height(0)
    ,threads(0)
    ,bytesPerRow(0)
    ,bufferSize(0)
    ,decoderFormat(kPRFormat_b64a)
    ,decoder(nullptr)
    ,outBuffer(nullptr)
    ,scratchMem(nullptr)
    ,pluginFormat(RGB48LE)
{
}

Status ProresDecApple::init(const ProresDecInitParams* initParams)
{
    close();
    msg.clear();
    for (int i = 0; i < initParams->count; i++)
    {
        if (GenericPlugin::setProperty(&initParams->properties[i]) == STATUS_OK)
            continue;

        std::string name(initParams->properties[i].name);
        std::string value(initParams->properties[i].value);

        if ("output_format" == name)
        {
            if (value == "yuv422p16le")
                pluginFormat = YUV422P16LE;
            else if (value == "rgb48le")
                pluginFormat = RGB48LE;
            else
            {
                msg += "\nInvalid 'format' value.";
                continue;
            }
        }
        else if ("width" == name)
        {
            int w = std::stoi(value);
            if (w < 0)
            {
                msg += "\nInvalid 'width' value.";
                continue;
            }
            this->width = w;
        }
        else if ("height" == name)
        {
            int h = std::stoi(value);
            if (h < 0)
            {
                msg += "\nInvalid 'height' value.";
                continue;
            }
            this->height = h;
        }
        else if ("multithread" == name)
        {
            int multithread = std::stoi(value);
            if (multithread < 0)
            {
                msg += "\nInvalid 'multithread' value.";
                continue;
            }
            this->threads = multithread;
        }
        else if ("thread_num" == name)
        {
            int thread_num = std::stoi(value);
            if (thread_num < 0)
            {
                msg += "\nInvalid 'thread_num' value.";
                continue;
            }
            this->threads = thread_num;
        }
        else
        {
            msg += "\nUnknown property: " + name;
        }
    }

    if (!msg.empty()) 
        return STATUS_ERROR;

    setPRPixelFormat();

    decoder = PROpenDecoder(threads, NULL);
    if (decoder == NULL)
    {
        msg += "\nFailed to open decoder.";
        return STATUS_ERROR;
    }

    bytesPerRow = PRBytesPerRowNeededInPixelBuffer(width, decoderFormat, kPRFullSize);
    bufferSize = height * bytesPerRow;
    decImgSize = bufferSize;
    outBuffer = new unsigned char[bufferSize];
    scratchMem = new uint8_t[bufferSize];

    makePixelBuffer();

    return STATUS_OK;
}

Status ProresDecApple::process(const ProresDecInput* in, ProresDecOutput* out)
{
    msg.clear();
    int bytes = PRDecodeFrame(decoder, in->buffer, (int)in->size, &pixelBuffer, kPRFullSize, false);
    bufferSize = decImgSize;

    if (bytes < 0) 
    {
        msg = "PRDecodeFrame returned 0 bytes.";
        return STATUS_ERROR;
    }

    if (convertDecodedPicture() != true)
        return STATUS_ERROR;

    out->buffer = outBuffer;
    out->size = bufferSize;
    out->width = pixelBuffer.width;
    out->height = pixelBuffer.height;
    out->format = pluginFormat;

    return STATUS_OK;
}

Status ProresDecApple::close()
{
    if (outBuffer) {
        delete[] outBuffer;
        outBuffer = nullptr;
    }
    if (scratchMem) {
        delete[] scratchMem;
        scratchMem = nullptr;
    }

    if (decoder) {
        PRCloseDecoder(decoder);
        decoder = nullptr;
    }
        
    return STATUS_OK;
}

Status ProresDecApple::setProperty(const Property*)
{
    return STATUS_ERROR;
}

Status ProresDecApple::getProperty(Property* property)
{
    if (NULL != property->name)
    {
        std::string name(property->name);
        if ("temp_file_num" == name)
        {
            strcpy(property->value, "0");
            return STATUS_OK;
        }
    }

    return STATUS_ERROR;
}

void ProresDecApple::fillProperties()
{
    if (propVector.empty())
    {
        auto genericProps = getGenericProperties();

        propVector.insert(propVector.end(), genericProps.begin(), genericProps.end());

        propVector.push_back({ "output_format", PROPERTY_TYPE_STRING, "Desired format of decoded data", "rgb48le", "yuv422p16le:rgb48le", 1, 1, ACCESS_TYPE_WRITE_INIT });
        propVector.push_back({ "width",  PROPERTY_TYPE_INTEGER, "Source picture width", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT });
        propVector.push_back({ "height", PROPERTY_TYPE_INTEGER, "Source picture height", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT });
        propVector.push_back({ "multithread", PROPERTY_TYPE_INTEGER, "Number of threads used for decoding. '0' means \"configure automatically\".", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT });
    }
}

PropertyInfo* ProresDecApple::getPropTable()
{
    return propVector.data();
}

int ProresDecApple::getPropTablesize()
{
    return (int)propVector.size();
}

const char* ProresDecPlugin::getName()
{
    return "apple";
}

ProresDecPlugin* ProresDecPlugin::createProresDecPluginInstance()
{
    return new ProresDecApple;
}

int ProresDecPlugin::getPluginProperties(const PropertyInfo** info)
{
    ProresDecApple::fillProperties();
    *info = ProresDecApple::getPropTable();
    return ProresDecApple::getPropTablesize();
}
