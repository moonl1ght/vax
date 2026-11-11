#include "descriptorRegistry.h"
#include "device.h"
#include "texture.h"

using namespace vax::vk;

DescriptorRegistry::RegistrationResult DescriptorRegistry::registerBatch(const RegistrationBatch& batch) noexcept {
    RegistrationResult result;
    std::vector<VkDescriptorImageInfo> storageImageInfos;
    std::vector<VkDescriptorImageInfo> sampledImageInfos;
    if (!batch.storageTextures.empty()) {
        result.storageTextureIndex =
            _registerTexture(batch.storageTextures, storageImageInfos, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, false);
    }
    if (!batch.sampledTextures.empty()) {
        result.sampledTextureIndex =
            _registerTexture(batch.sampledTextures, sampledImageInfos, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, false);
    }

    vkUpdateDescriptorSets(_device.get().vkDevice(), static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);
    return result;
}

std::optional<uint32_t>
DescriptorRegistry::registerStorageTexture(const std::vector<const Texture*>& textures) noexcept {
    std::vector<VkDescriptorImageInfo> imageInfos;
    return _registerTexture(textures, imageInfos, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, true);
}

std::optional<uint32_t>
DescriptorRegistry::registerSampledTexture(const std::vector<const Texture*>& textures) noexcept {
    std::vector<VkDescriptorImageInfo> imageInfos;
    return _registerTexture(textures, imageInfos, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, true);
}

std::optional<uint32_t> DescriptorRegistry::_registerTexture(
    const std::vector<const Texture*>& textures,
    std::vector<VkDescriptorImageInfo>& imageInfos,
    VkDescriptorType descriptorType,
    uint32_t binding,
    bool update
) noexcept {
    imageInfos.reserve(textures.size());
    for (const auto& texture : textures) {
        auto imageInfoOpt = texture->descriptorImageInfoNoSampler();
        if (!imageInfoOpt) {
            return std::nullopt;
        }
        imageInfos.push_back(*imageInfoOpt);
    }
    auto index = _sampledTextures.allocate(imageInfos.size());
    if (!index) {
        return std::nullopt;
    }

    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSet,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = static_cast<uint32_t>(imageInfos.size()),
        .descriptorType = descriptorType,
        .pImageInfo = imageInfos.data()
    };
    if (update) {
        vkUpdateDescriptorSets(_device.get().vkDevice(), 1, &write, 0, nullptr);
    } else {
        _writes.push_back(write);
    }
    return index;
}