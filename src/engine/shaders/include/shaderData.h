#ifndef shaderUniforms_h
#define shaderUniforms_h

#ifdef __cplusplus

#include <glm/ext/matrix_float3x4.hpp>
#include <glm/glm.hpp>

using mat4 = glm::mat4;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;

#else // Slang

typedef float4x4 mat4;
typedef float3x3 mat3;
typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;

#endif

#include "shaderConstants.h"

enum ObjectFlags {
    NoFlags = 0,
    IsWireframe = 1 << 0,    // 0001
    NoTangent = 1 << 1,      // 0010
    PrecomputedMVP = 1 << 2, // 0100
};

enum InstanceFlags {
    InstanceFlagsNone = 0,
    IsInstanceSelected = 1 << 0, // 0001
};

struct EnvironmentMapData {
    uint32_t envMapTexture;
    uint32_t envMapTextureSampler;

    uint32_t envMapTextureIrradiance;
    uint32_t envMapTextureIrradianceSampler;

    uint32_t texBRDFLUT;
    uint32_t texBRDFLUTSampler;

    uint32_t padding[2];
};

struct UniformBufferObject {                                 // total size: 160 bytes
    mat4 view;                                               // 64 bytes
    mat4 proj;                                               // 64 bytes
    vec4 cameraPosition;                                     // 16 bytes
    uint32_t environmentMapIndex = NO_ENVIRONMENT_MAP_INDEX; // 4 bytes
    vec3 padding;                                            // 12 bytes
};

struct LightData {                  // total size: 112 bytes
    mat4 lightSpaceMatrix;          // 64 bytes
    vec4 position;                  // 16 bytes
    vec4 color;                     // 16 bytes
    uint32_t shadowMapIndex;        // 4 bytes
    uint32_t shadowMapSamplerIndex; // 4 bytes
    float radius;                   // 4 bytes
    float padding;                  // 4 bytes
};

struct LightUBO { // total size: 112 * MAX_UBO_LIGHTS bytes = 1792 bytes + 16 bytes = 1808 bytes
    LightData lights[MAX_UBO_LIGHTS];
    uint32_t lightCount; // 4 bytes
    uint32_t padding[3]; // 12 bytes
};

struct InstanceData {     // total size: 144 bytes
    mat4 model;           // 64 bytes
    mat4 normalMatrix;    // 64 bytes
    uint32_t packedColor; // 4 bytes
    uint32_t flags;       // 4 bytes
    uint32_t instanceId;  // 4 bytes
    uint32_t padding;     // 4 bytes
};

struct PerDrawData {
    uint32_t flags = 0;                         // 4 bytes
    uint32_t materialIndex = NO_MATERIAL_INDEX; // 4 bytes
};

struct GlobalPushConstants {
    uint32_t drawIndexOffset;
};

#endif // shaderUniforms_h