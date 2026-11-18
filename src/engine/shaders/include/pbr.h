#ifndef PBR_H
#define PBR_H

#include "shaderData.h"
#include "shaderResourceHandle.h"
#include "vertex.h"

struct PBRMaterial {
    vec4 baseColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float metallicFactor = 0.0f, roughnessFactor = 1.0f, normalScale = 1.0f, occlusionStrength = 1.0f; // vec4
    vec4 emissiveFactorAlphaCutoff = vec4(0.0f, 0.0f, 0.0f, 0.5f);

    uint32_t baseColorTextureIndex = NO_TEXTURE_FLAG;
    uint32_t baseColorTextureSamplerIndex = NO_SAMPLER_INDEX;
    uint32_t baseColorTextureUVIndex = 0;

    uint32_t normalMapTextureIndex = NO_TEXTURE_FLAG;
    uint32_t normalMapTextureSamplerIndex = NO_SAMPLER_INDEX;
    uint32_t normalMapTextureUVIndex = 0;

    uint32_t metallicRoughnessTextureIndex = NO_TEXTURE_FLAG;
    uint32_t metallicRoughnessTextureSamplerIndex = NO_SAMPLER_INDEX;
    uint32_t metallicRoughnessTextureUVIndex = 0;

    uint32_t aoTextureIndex = NO_TEXTURE_FLAG;
    uint32_t aoTextureSamplerIndex = NO_SAMPLER_INDEX;
    uint32_t aoTextureUVIndex = 0;

    uint32_t emissiveTextureIndex = NO_TEXTURE_FLAG;
    uint32_t emissiveTextureSamplerIndex = NO_SAMPLER_INDEX;
    uint32_t emissiveTextureUVIndex = 0;

    uint32_t alphaMode = 0;
};

struct PBRPushConstants {
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<Vertex>) vertices;
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<InstanceData>) instances;
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<PerDrawData>) drawData;
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<PBRMaterial>) materials;
    STRUCTURED_BUFFER_HANDLE(StructuredBuffer<EnvironmentMapData>) environmentMaps;
    uint32_t drawIndexOffset;
};

#endif