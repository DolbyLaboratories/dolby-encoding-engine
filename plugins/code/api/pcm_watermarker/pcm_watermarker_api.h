#ifndef __DEE_PLUGINS_PCM_WATERMARKER_API_H__
#define __DEE_PLUGINS_PCM_WATERMARKER_API_H__
 
#include "plugins_common.h"
#include <cstdint>

#define PCM_WATERMARKER_API_VERSION 1
 
#ifdef __cplusplus
extern "C" {
#endif
 
struct PcmWatermarkerFrame{
    PcmWatermarkerFrame() {
        buffer = nullptr;
        channelsCount = 0;
        samplesCount = 0;
        stride = 0;
    }

    PcmWatermarkerFrame(float** buffer, uint32_t channelsCount, uint64_t samplesCount, uint32_t stride) {
        buffer = buffer;
        channelsCount = channelsCount;
        samplesCount = samplesCount;
        stride = stride;
    }

    float** buffer;
    uint32_t channelsCount;
    uint64_t samplesCount;
    uint32_t stride;
};
 
typedef struct {
    const Property* properties;
    size_t count;
} PcmWatermarkerInitParams;
 
typedef void* PcmWatermarkerHandle;
 
typedef size_t (*PcmWatermarkerGetInfo)(const PropertyInfo** info);
 
typedef size_t (*PcmWatermarkerGetSize)();
 
typedef Status (*PcmWatermarkerInit)(PcmWatermarkerHandle handle, const PcmWatermarkerInitParams* initParams);
 
typedef Status (*PcmWatermarkerClose)(PcmWatermarkerHandle handle);
 
typedef Status (*PcmWatermarkerProcess) (PcmWatermarkerHandle handle, PcmWatermarkerFrame* inFrame);
 
typedef const char* (*PcmWatermarkerGetMessage)(PcmWatermarkerHandle handle);
 
typedef struct {
    const char* pluginName;
    PcmWatermarkerGetInfo getInfo;
    PcmWatermarkerGetSize getSize;
    PcmWatermarkerInit init;
    PcmWatermarkerClose close;
    PcmWatermarkerProcess process;
    PcmWatermarkerGetMessage getMessage;
} PcmWatermarkerApi;
 
DLB_EXPORT
PcmWatermarkerApi* pcmWatermarkerGetApi();
 
typedef PcmWatermarkerApi* (*PcmWatermarkerGetApi)();
 
DLB_EXPORT
int pcmWatermarkerGetApiVersion(void);
 
typedef int (*PcmWatermarkerGetApiVersion)(void);
 
#ifdef __cplusplus
}
#endif

#endif