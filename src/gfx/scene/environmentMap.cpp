#include "environmentMap.h"

using namespace vax::gfx;
using namespace vax;

void EnvironmentMap::cleanup() noexcept {
    _descriptorRegistry.get().releaseStorageBuffer(_bufferResource->first);
    _bufferResource = nullptr;
}

bool EnvironmentMap::load(
    const Descriptor& descriptor,
    const rhi::SamplerManager& samplerManager,
    rhi::TextureLibrary& textureLibrary,
    rhi::TextureLoader& textureLoader,
    vax::rhi::TextureGpuUploader& textureGpuUploader
) noexcept {
    auto cubeMapSampler = samplerManager.getGlobalSampler(rhi::SamplerManager::GlobalSampler::CubeMapSampler);
    if (!cubeMapSampler) {
        _logger.error("Cube map sampler is missing!");
        return false;
    }
    _environmentMapData = EnvironmentMapData{
        .envMapTexture = NO_TEXTURE_FLAG,
        .envMapTextureSampler = cubeMapSampler->shaderHandle.index,
        .envMapTextureIrradiance = NO_TEXTURE_FLAG,
        .envMapTextureIrradianceSampler = cubeMapSampler->shaderHandle.index,
        .texBRDFLUT = NO_TEXTURE_FLAG,
        .texBRDFLUTSampler = cubeMapSampler->shaderHandle.index,
    };
    for (const auto& textureConfig : descriptor.textures) {
        auto texture = textureLoader.loadTexture(textureConfig.second);
        textureGpuUploader.addTexture(texture.get());
        if (!texture) {
            _logger.error("Failed to load texture: {}", textureConfig.second);
            continue;
        }
        switch (textureConfig.first) {
        case TextureType::EnvMap:
            _environmentMapData.envMapTexture = texture->sampledShaderHandle().index;
            break;
        case TextureType::EnvMapIrradiance:
            _environmentMapData.envMapTextureIrradiance = texture->sampledShaderHandle().index;
            break;
        case TextureType::BRDFLUT:
            _environmentMapData.texBRDFLUT = texture->sampledShaderHandle().index;
            break;
        }
        textureLibrary.addTexture(std::move(texture));
    }
    VkDeviceSize bufferSize = sizeof(EnvironmentMapData);
    auto allocation = EnvironmentMapBuffer::allocate(
        _device.get(),
        "environment_map_buffer",
        bufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );
    if (!allocation.has_value()) {
        _logger.error("Failed to allocate environment map buffer!");
        return false;
    }
    allocation->map();
    auto mappedMemory = allocation->mappedMemory();
    if (!mappedMemory.has_value()) {
        _logger.error("Failed to map environment map buffer!");
        return false;
    }
    EnvironmentMapData* environmentMapDataPtr = mappedMemory.value();
    *environmentMapDataPtr = _environmentMapData;
    auto handle = _descriptorRegistry.get().registerStorageBuffer(*allocation);
    if (!handle.has_value()) {
        _logger.error("Failed to register environment map buffer!");
        return false;
    }
    _bufferResource = std::make_unique<EnvironmentMapBufferResoruce>(*handle, std::move(*allocation));
    return true;
}