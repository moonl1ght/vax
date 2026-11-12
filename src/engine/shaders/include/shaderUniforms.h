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

#include "shaderResourceHandle.h"

enum MainSetIndices {
    GLOBAL_SET_INDEX = 0,
    PER_FRAME_SET_INDEX = 1,
    PER_DRAW_SET_INDEX = 2,
};

enum PerFrameDescriptorSetResourceIndex {
    FRAME_UNIFORM_BUFFER_INDEX_ = 0,
    FRAME_LIGHT_BUFFER_INDEX = 1,
    FRAME_INSTANCE_BUFFER_INDEX = 2,
};

enum DrawBindingIndices {
    DRAW_DATA_BUFFER_INDEX = 0,
};

enum GlobalDescriptorSetResourceIndex {
    GLOBAL_MATERIAL_BUFFER_INDEX = 0,
    GLOBAL_ENVIRONMENT_MAP_BUFFER_INDEX = 1,
    GLOBAL_SAMPLER_INDEX = 2,
    GLOBAL_TEXTURE_INDEX = 3,
    GLOBAL_SHADOW_TEXTURE_INDEX = 4,
    GLOBAL_VERTEX_BUFFER_INDEX = 5,
};

enum VertexInputIndicesPUV {
    VERTEX_INPUT_PUV_POSITION_INDEX = 0,
    VERTEX_INPUT_PUV_UV_INDEX = 1,
};

enum VertexInputIndices {
    VERTEX_INPUT_POSITION_INDEX = 0,
    VERTEX_INPUT_PACKED_COLOR_INDEX = 1,
    VERTEX_INPUT_TANGENT_INDEX = 2,
    VERTEX_INPUT_NORMAL_INDEX = 3,
    VERTEX_INPUT_UV_INDEX = 4,
    VERTEX_INPUT_UV_2_INDEX = 5,
};

enum VertexInputIndicesNoTangent {
    VERTEX_INPUT_COLOR_INDEX_NT = 0,
    VERTEX_INPUT_POSITION_INDEX_NT = 1,
    VERTEX_INPUT_NORMAL_INDEX_NT = 2,
    VERTEX_INPUT_UV_INDEX_NT = 3,
    VERTEX_INPUT_UV_2_INDEX_NT = 4
};

enum {
    MAX_TEXTURES = 500,
    MAX_CUBE_MAP_TEXTURES = 10,
    MAX_SHADOW_TEXTURES = 100,
    MAX_SAMPLERS_ = 4,
    MAX_GLOBAL_SAMPLERS = MAX_SAMPLERS_,
    MAX_UBO_LIGHTS = 16
};

static constexpr uint32_t NO_TEXTURE_FLAG = 0xFFFFFFFF;
static constexpr uint32_t NO_MATERIAL_INDEX = 0xFFFFFFFF;
static constexpr uint32_t NO_ENVIRONMENT_MAP_INDEX = 0xFFFFFFFF;
static constexpr uint32_t NO_SAMPLER_INDEX = 0xFFFFFFFF;
static constexpr uint32_t NO_SHADOW_MAP_INDEX = 0xFFFFFFFF;
static constexpr uint32_t NO_ID = 0xFFFFFFFF;

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

#endif // shaderUniforms_h