module sampling;

import sharedTypes;
import layout;

Texture2D.Handle textureHandle(uint index) { return Texture2D.Handle(uint2(index, 0)); }

TextureCube.Handle cubeTextureHandle(uint index) { return TextureCube.Handle(uint2(index, 0)); }

SamplerState.Handle samplerHandle(uint index) { return SamplerState.Handle(uint2(index, 0)); }

float4 sampleTexture(uint textureIndex, uint samplerIndex, float2 uv) {
    return textureHandle(textureIndex).Sample(samplerHandle(samplerIndex), uv);
}

public float4 sampleBaseColor(PBRMaterial material, float2 uv) {
    if (material.baseColorTextureIndex == NO_TEXTURE_FLAG)
        return material.baseColor;
    return sampleTexture(material.baseColorTextureIndex, material.baseColorTextureSamplerIndex, uv);
}

public float4 sampleAmbientOcclusion(PBRMaterial material, float2 uv) {
    if (material.aoTextureIndex == NO_TEXTURE_FLAG)
        return float4(1.0f, 1.0f, 1.0f, 1.0f);
    return sampleTexture(material.aoTextureIndex, material.aoTextureSamplerIndex, uv);
}

public float4 sampleMetallicRoughness(PBRMaterial material, float2 uv) {
    if (material.metallicRoughnessTextureIndex == NO_TEXTURE_FLAG)
        return float4(1.0f, 1.0f, 1.0f, 1.0f);
    return sampleTexture(material.metallicRoughnessTextureIndex, material.metallicRoughnessTextureSamplerIndex, uv);
}

public float4 sampleNormalMap(PBRMaterial material, float2 uv) {
    if (material.normalMapTextureIndex == NO_TEXTURE_FLAG)
        return float4(0.5f, 0.5f, 1.0f, 1.0f);
    return sampleTexture(material.normalMapTextureIndex, material.normalMapTextureSamplerIndex, uv);
}

public float4 sampleEmissive(PBRMaterial material, float2 uv) {
    if (material.emissiveTextureIndex == NO_TEXTURE_FLAG)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    return sampleTexture(material.emissiveTextureIndex, material.emissiveTextureSamplerIndex, uv);
}

public int sampleEnvMapQueryLevels(EnvironmentMapData environmentMapData) {
    uint width, height, levels;
    cubeTextureHandle(environmentMapData.envMapTexture).GetDimensions(0, width, height, levels);
    return int(levels);
}

public float4 sampleBRDFLUT(float2 brdfSamplePoint, EnvironmentMapData environmentMapData) {
    return sampleTexture(environmentMapData.texBRDFLUT, environmentMapData.texBRDFLUTSampler, brdfSamplePoint);
}

public float4 sampleEnvMapLod(float3 reflection, float lod, EnvironmentMapData environmentMapData) {
    return cubeTextureHandle(environmentMapData.envMapTexture)
        .SampleLevel(samplerHandle(environmentMapData.envMapTextureSampler), reflection, lod);
}

public float4 sampleEnvMapIrradiance(float3 normal, EnvironmentMapData environmentMapData) {
    return cubeTextureHandle(environmentMapData.envMapTextureIrradiance)
        .Sample(samplerHandle(environmentMapData.envMapTextureIrradianceSampler), normal);
}
