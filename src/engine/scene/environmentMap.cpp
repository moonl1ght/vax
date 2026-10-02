#include "environmentMap.h"

using namespace vax::engine;
using namespace vax;

void EnvironmentMap::load(const Descriptor& descriptor) {
    _environmentMapData = EnvironmentMapData{
        .envMapTexture = NO_TEXTURE_FLAG,
        .envMapTextureSampler = static_cast<uint32_t>(vk::GlobalSampler::CubeMapSampler),
        .envMapTextureIrradiance = NO_TEXTURE_FLAG,
        .envMapTextureIrradianceSampler = static_cast<uint32_t>(vk::GlobalSampler::CubeMapSampler),
        .texBRDFLUT = NO_TEXTURE_FLAG,
        .texBRDFLUTSampler = static_cast<uint32_t>(vk::GlobalSampler::CubeMapSampler),
    };
    for (const auto& texture : descriptor.textures) {
        auto textureResource = _textureLoader->loadTexture(texture.second);
        if (!textureResource.has_value()) {
            _logger.error("Failed to load texture: {}", texture.second);
            continue;
        }
        switch (texture.first) {
            case TextureType::EnvMap:
                _environmentMapData.envMapTexture = textureResource->first.id();
                break;
            case TextureType::EnvMapIrradiance:
                _environmentMapData.envMapTextureIrradiance = textureResource->first.id();
                break;
            case TextureType::BRDFLUT:
                _environmentMapData.texBRDFLUT = textureResource->first.id();
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
    if (!allocation.has_value())
        return;
    _buffer = std::make_unique<EnvironmentMapBuffer>(std::move(*allocation));
    _buffer->map();
    auto mappedMemory = _buffer->mappedMemory();
    if (!mappedMemory.has_value())
        return;
    EnvironmentMapData* environmentMapDataPtr = mappedMemory.value();
    *environmentMapDataPtr = _environmentMapData;
}