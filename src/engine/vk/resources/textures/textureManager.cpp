#include "textureManager.h"
#include "imageUtils.h"

using namespace vax;
using namespace vax::vk;

bool TextureManager::setup() noexcept {
    auto globalSampler = Sampler::linear(_device.get());
    auto globalCubeMapSampler = Sampler::cube(_device.get());
    if (!globalSampler.has_value() || !globalCubeMapSampler.has_value()) {
        _logger.error("Failed to create global samplers");
        return false;
    }
    auto linearSamplerHandle = _descriptorRegistry.get().registerSampler(globalSampler.value().vkSampler());
    auto cubeMapSamplerHandle = _descriptorRegistry.get().registerSampler(globalCubeMapSampler.value().vkSampler());
    if (!linearSamplerHandle.has_value() || !cubeMapSamplerHandle.has_value()) {
        _logger.error("Failed to register global samplers");
        return false;
    }
    SamplerResource linearSamplerResource{
        .sampler = std::move(globalSampler.value()),
        .shaderHandle = *linearSamplerHandle,
    };
    _globalSamplers.emplace_back(std::move(linearSamplerResource));
    SamplerResource cubeMapSamplerResource{
        .sampler = std::move(globalCubeMapSampler.value()),
        .shaderHandle = *cubeMapSamplerHandle,
    };
    _globalSamplers.emplace_back(std::move(cubeMapSamplerResource));
    return true;
}

void TextureManager::cleanup() noexcept {
    for (auto& textureResource : _textures) {
        if (textureResource->registrationType == TextureRegistrationType::SampledImage) {
            _descriptorRegistry.get().releaseSampledTexture(textureResource->sampledShaderHandle);
        } else if (textureResource->registrationType == TextureRegistrationType::StorageImage) {
            _descriptorRegistry.get().releaseStorageTexture(textureResource->storageShaderHandle);
        }
        textureResource->texture._destroy();
    }
    _textures.clear();
}

TextureManager::TextureResource* TextureManager::findTexture(TextureId id) noexcept {
    if (id >= _textures.size()) {
        _logger.warning("Texture id not found");
        return nullptr;
    }
    return _textures[id].get();
}

const TextureManager::SamplerResource* TextureManager::getGlobalSampler(GlobalSampler sampler) const noexcept {
    auto index = static_cast<uint32_t>(sampler);
    if (index >= _globalSamplers.size()) {
        _logger.error("Global sampler index out of bounds");
        return nullptr;
    }
    return &_globalSamplers[index];
}

void TextureManager::loadAllTextures(vax::vk::CommandBuffer& commandBuffer) noexcept {
    for (auto& textureResource : _textures) {
        textureResource->texture.loadStagingBuffer(commandBuffer);
    }
}

void TextureManager::resetAllTexturesStagingBuffers() noexcept {
    for (auto& textureResource : _textures) {
        textureResource->texture.resetStagingBuffer();
    }
}

TextureManager::TextureResource* TextureManager::createDepthTexture(
    VkFormat format, vax::math::SizeUI size, TextureRegistrationType registrationType
) noexcept {
    auto imageResult = vax::vk::createImage(
        _device.get().allocator(),
        size.toExtent3D(),
        format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    if (!imageResult) {
        _logger.error("Failed to create depth texture!");
        return nullptr;
    }
    VkDebugUtilsObjectNameInfoEXT nameInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = VK_OBJECT_TYPE_IMAGE,
        .objectHandle = reinterpret_cast<size_t>(imageResult->first),
        .pObjectName = "depth_texture",
    };
    vax::vk::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice(), &nameInfo);
    auto [depthImage, allocation] = imageResult.value();
    _textures.push_back(
        std::unique_ptr<TextureResource>(new TextureResource{
        .id = static_cast<TextureId>(_textures.size()),
        .registrationType = registrationType,
        .texture =
            Texture(_device.get(), "depth_texture", depthImage, allocation, size, format, VK_IMAGE_ASPECT_DEPTH_BIT),
        })
    );
    if (!_registerLastTexture()) {
        _logger.error("Failed to register last texture");
        return nullptr;
    }
    return _textures.back().get();
}

TextureManager::TextureResource* TextureManager::createTexture(const TextureCreateInfo& createInfo) noexcept {
    auto imageResult = createImage(
        _device.get().allocator(),
        createInfo.size.toExtent3D(),
        createInfo.format,
        VK_IMAGE_TILING_OPTIMAL,
        createInfo.imageUsageFlags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        createInfo.numLayers,
        createInfo.numMips,
        createInfo.flags
    );
    if (!imageResult) {
        _logger.error("Failed to create texture!");
        return nullptr;
    }
    auto [image, allocation] = imageResult.value();
    if (!createInfo.name.empty()) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_IMAGE,
            .objectHandle = reinterpret_cast<size_t>(image),
            .pObjectName = createInfo.name.c_str(),
        };
        vax::vk::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice(), &nameInfo);
    }
    _textures.push_back(
        std::unique_ptr<TextureResource>(new TextureResource{
        .id = static_cast<TextureId>(_textures.size()),
        .registrationType = createInfo.registrationType,
        .texture = Texture(
            _device.get(),
            createInfo.name,
            image,
            allocation,
            createInfo.size,
            createInfo.format,
            VK_IMAGE_ASPECT_COLOR_BIT
        )
        })
    );
    if (!_registerLastTexture()) {
        _logger.error("Failed to register last texture");
        return nullptr;
    }
    return _textures.back().get();
}

bool TextureManager::_registerLastTexture() noexcept {
    auto& textureResource = _textures.back();
    switch (textureResource->registrationType) {
    case TextureRegistrationType::SampledImage:
        return _registerLastSampledTexture();
    case TextureRegistrationType::StorageImage:
        return _registerLastStorageTexture();
    case TextureRegistrationType::Both:
        return _registerLastSampledTexture() && _registerLastStorageTexture();
    }
    return false;
}

bool TextureManager::_registerLastStorageTexture() noexcept {
    auto& textureResource = _textures.back();
    auto textureHandles = _descriptorRegistry.get().registerStorageTextures({&textureResource->texture}, true);
    if (!textureHandles || textureHandles->empty()) {
        _logger.error("Failed to register storage texture");
        return false;
    }
    return true;
}

bool TextureManager::_registerLastSampledTexture() noexcept {
    auto& textureResource = _textures.back();
    auto textureHandles = _descriptorRegistry.get().registerSampledTextures({&textureResource->texture}, true);
    if (!textureHandles || textureHandles->empty()) {
        _logger.error("Failed to register sampled texture");
        return false;
    }
    return true;
}