#ifndef jfa_h
#define jfa_h

#include "shaderResourceHandle.h"
#include "shaderConstants.h"

struct JFAInitPushConstants {
    SAMPLED_IMAGE_HANDLE(Texture2D<uint>) inputMask;
    SAMPLED_IMAGE_HANDLE(Texture2D<float>) inputDepth;
    STORAGE_IMAGE_HANDLE(RWTexture2D<uint2>) output;
};

struct JFAStepPushConstants {
    STORAGE_IMAGE_HANDLE(RWTexture2D<uint2>) input;
    STORAGE_IMAGE_HANDLE(RWTexture2D<uint2>) output;
    uint step;
};

#endif