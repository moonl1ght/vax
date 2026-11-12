#ifndef finalblend_h
#define finalblend_h

#include "shaderResourceHandle.h"
#include "shaderUniforms.h"

struct FinalBlendPushConstants {
    SAMPLER_HANDLE(SamplerState) inputTextureSampler;
    SAMPLED_IMAGE_HANDLE(Texture2D<float4>) inputTexture;
    SAMPLED_IMAGE_HANDLE(Texture2D<uint>) inputMask;
    SAMPLED_IMAGE_HANDLE(Texture2D<float>) inputDepth;
    STORAGE_IMAGE_HANDLE(RWTexture2D<uint2>) output;
    SAMPLED_IMAGE_HANDLE(Texture2D<uint2>) inputJFATexture;
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<InstanceData>) instances;
};

#endif