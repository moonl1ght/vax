#include "environmentMap.h"

using namespace vax::gfx;
using namespace vax;

bool EnvironmentMap::load(const Descriptor& descriptor) noexcept {
    auto cubeMapSampler =
        _resourceManager.get().textureManager().getGlobalSampler(rhi::TextureManager::GlobalSampler::CubeMapSampler);
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
    for (const auto& texture : descriptor.textures) {
        auto textureResource = _textureLoader->loadTexture(texture.second);
        if (!textureResource) {
            _logger.error("Failed to load texture: {}", texture.second);
            continue;
        }
        switch (texture.first) {
        case TextureType::EnvMap:
            _environmentMapData.envMapTexture = textureResource->sampledShaderHandle.index;
            break;
        case TextureType::EnvMapIrradiance:
            _environmentMapData.envMapTextureIrradiance = textureResource->sampledShaderHandle.index;
            break;
        case TextureType::BRDFLUT:
            _environmentMapData.texBRDFLUT = textureResource->sampledShaderHandle.index;
            break;
        }
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
    _buffer = std::make_unique<EnvironmentMapBuffer>(std::move(*allocation));
    _buffer->map();
    auto mappedMemory = _buffer->mappedMemory();
    if (!mappedMemory.has_value()) {
        _logger.error("Failed to map environment map buffer!");
        return false;
    }
    EnvironmentMapData* environmentMapDataPtr = mappedMemory.value();
    *environmentMapDataPtr = _environmentMapData;
    return true;
}