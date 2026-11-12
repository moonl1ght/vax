#ifndef jfa_h
#define jfa_h

#ifdef __cplusplus

#include <glm/ext/matrix_float3x4.hpp>
#include <glm/glm.hpp>

using mat4 = glm::mat4;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;

#endif

#include "shaderResourceHandle.h"

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

static constexpr uint MAX_UINT = 60000;

#endif