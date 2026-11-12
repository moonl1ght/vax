import uniforms;
import utils.customAttributes;

struct BindlessDescriptorSet {
    Texture2D textures[MAX_TEXTURES];
};

[DynamicOffset]
[vk::binding(PerFrameDescriptorSetResourceIndex::FRAME_UNIFORM_BUFFER_INDEX, MainSetIndices::PER_FRAME_SET_INDEX)]
ConstantBuffer<UniformBufferObject> ubo;

[vk::binding(PerFrameDescriptorSetResourceIndex::FRAME_LIGHT_BUFFER_INDEX, MainSetIndices::PER_FRAME_SET_INDEX)]
ConstantBuffer<LightUBO> lights;

[vk::binding(PerFrameDescriptorSetResourceIndex::FRAME_INSTANCE_BUFFER_INDEX, MainSetIndices::PER_FRAME_SET_INDEX)]
StructuredBuffer<InstanceData> instances;

[vk::binding(DrawBindingIndices::DRAW_DATA_BUFFER_INDEX, MainSetIndices::PER_DRAW_SET_INDEX)]
StructuredBuffer<PerDrawData> drawData;

[vk::binding(GlobalDescriptorSetResourceIndex::GLOBAL_MATERIAL_BUFFER_INDEX, MainSetIndices::GLOBAL_SET_INDEX)]
StructuredBuffer<PBRMaterial> materials;

[vk::binding(GlobalDescriptorSetResourceIndex::GLOBAL_ENVIRONMENT_MAP_BUFFER_INDEX, MainSetIndices::GLOBAL_SET_INDEX)]
StructuredBuffer<EnvironmentMapData> environmentMaps;

[vk::binding(GlobalDescriptorSetResourceIndex::GLOBAL_SAMPLER_INDEX, MainSetIndices::GLOBAL_SET_INDEX)]
SamplerState globalSamplers[MAX_SAMPLERS];

[vk::binding(GlobalDescriptorSetResourceIndex::GLOBAL_TEXTURE_INDEX, MainSetIndices::GLOBAL_SET_INDEX)]
Texture2D globalTextures[MAX_TEXTURES];

[vk::binding(GlobalDescriptorSetResourceIndex::GLOBAL_TEXTURE_INDEX, MainSetIndices::GLOBAL_SET_INDEX)]
TextureCube globalCubeTextures[MAX_CUBE_MAP_TEXTURES];

[vk::binding(GlobalDescriptorSetResourceIndex::GLOBAL_SHADOW_TEXTURE_INDEX, MainSetIndices::GLOBAL_SET_INDEX)]
Texture2D globalShadowTextures[MAX_SHADOW_TEXTURES];

struct VertexInput {
    float3 position;

    uint packedColor;

    float4 tangent;

    float3 normal;

    uint padding;

    float2 uv;

    float2 uv2;
};

[vk::binding(GlobalDescriptorSetResourceIndex::GLOBAL_VERTEX_BUFFER_INDEX, MainSetIndices::GLOBAL_SET_INDEX)]
StructuredBuffer<VertexInput> vertices;

struct VertexOutput {
    float4 position : SV_Position;

    float4 tangent : TANGENT;

    float3 worldPosition : WORLD_POSITION;

    float3 normal : NORMAL;

    float2 uv : TEXCOORD0;

    float2 uv2 : TEXCOORD1;

    uint packedColor : PACKED_COLOR;

    nointerpolation uint instanceIndex : INSTANCE_INDEX;

    nointerpolation uint drawIndex : DRAW_INDEX;
};

