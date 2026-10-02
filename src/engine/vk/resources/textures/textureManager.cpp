#include "textureManager.h"
#include "descriptorSetWriter.h"
#include "textureFactory.h"

using namespace vax;
using namespace vax::vk;

bool TextureManager::setup() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(_device.get().vkPhysicalDevice, &properties);
    VkSamplerCreateInfo globalSamplerInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    VkSamplerCreateInfo globalCubeMapSamplerInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    auto globalSampler = Sampler::createSampler(_device.get(), "global_sampler", globalSamplerInfo);
    auto globalCubeMapSampler =
        Sampler::createSampler(_device.get(), "global_cube_map_sampler", globalCubeMapSamplerInfo);
    if (!globalSampler.has_value() || !globalCubeMapSampler.has_value()) {
        _logger.error("Failed to create global sampler");
        return false;
    }
    _globalSamplers.push_back(std::move(*globalSampler));
    _globalSamplers.push_back(std::move(*globalCubeMapSampler));
    return true;
}

void TextureManager::fullCleanup() {
    for (auto& [id, texture] : _pool) {
        texture._destroy();
    }
    _pool.clear();
}

TextureFactory TextureManager::createTextureFactory() const {
    return TextureFactory(_device.get(), const_cast<TextureManager* const>(this));
}

std::optional<TextureManager::TextureResource> TextureManager::attach(Texture&& texture) {
    texture._id = _lastId++;
    auto [it, inserted] = _pool.try_emplace(texture.id(), std::move(texture));
    if (!inserted) {
        _logger.error("Failed to attach texture to manager");
        return std::nullopt;
    }
    it->second._isDetached = false;
    return std::make_pair(it->first, &it->second);
}

std::optional<TextureManager::TextureResource> TextureManager::find(TextureHandle handle) {
    auto it = _pool.find(handle.id());
    if (it == _pool.end())
        return std::nullopt;
    return std::make_pair(handle, &it->second);
}

bool TextureManager::deleteTexture(TextureHandle handle) {
    auto it = _pool.find(handle.id());
    if (it == _pool.end())
        return false;
    it->second._destroy();
    _pool.erase(it);
    return true;
}

std::optional<Texture> TextureManager::detach(TextureHandle handle) {
    auto it = _pool.find(handle.id());
    if (it == _pool.end())
        return std::nullopt;
    it->second._isDetached = true;
    _pool.erase(it);
    return std::move(it->second);
}

void TextureManager::updateDescriptorWriterWithAllTextures(
    vax::vk::DescriptorSetWriter& descriptorWriter, uint32_t binding
) const {
    std::vector<const Texture*> textures(_pool.size());
    for (auto& [id, texture] : _pool) {
        textures[texture.id()] = &texture;
    }

    if (textures.empty()) {
        return;
    }
    descriptorWriter.writeTextures(textures, binding);
}

std::optional<TextureManager::SamplerResource> TextureManager::getGlobalSampler(GlobalSampler sampler) {
    auto index = static_cast<uint32_t>(sampler);
    if (index >= _globalSamplers.size()) {
        _logger.error("Global sampler index out of bounds");
        return std::nullopt;
    }
    return std::make_pair(SamplerHandle(index), &_globalSamplers[index]);
}

void TextureManager::loadAllTextures(vax::vk::CommandBuffer& commandBuffer) {
    for (auto& [id, texture] : _pool) {
        texture.loadStagingBuffer(commandBuffer);
    }
}

void TextureManager::resetAllTexturesStagingBuffers() {
    for (auto& [id, texture] : _pool) {
        texture.resetStagingBuffer();
    }
}