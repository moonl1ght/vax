#include "descriptorRegistry.h"
#include "device.h"
#include "texture.h"

using namespace vax::vk;

DescriptorRegistry::RegistrationResult DescriptorRegistry::registerBatch(const RegistrationBatch& batch) noexcept {
    RegistrationResult result;
    std::vector<VkDescriptorImageInfo> storageImageInfos;
    std::vector<VkDescriptorImageInfo> sampledImageInfos;
    if (!batch.storageTextures.empty()) {
        result.storageTextures = _toHandles<StorageHandle>(_registerTexture(
            batch.storageTextures,
            storageImageInfos,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            _storageTextures,
            1,
            false,
            batch.contiguous
        ));
    }
    if (!batch.sampledTextures.empty()) {
        result.sampledTextures = _toHandles<SampledHandle>(_registerTexture(
            batch.sampledTextures,
            sampledImageInfos,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            _sampledTextures,
            0,
            false,
            batch.contiguous
        ));
    }

    vkUpdateDescriptorSets(_device.get().vkDevice(), static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);
    _writes.clear();
    return result;
}

std::optional<std::vector<StorageHandle>>
DescriptorRegistry::registerStorageTexture(const std::vector<const Texture*>& textures, bool contiguous) noexcept {
    std::vector<VkDescriptorImageInfo> imageInfos;
    return _toHandles<StorageHandle>(
        _registerTexture(textures, imageInfos, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, _storageTextures, 1, true, contiguous)
    );
}

std::optional<std::vector<SampledHandle>>
DescriptorRegistry::registerSampledTexture(const std::vector<const Texture*>& textures, bool contiguous) noexcept {
    std::vector<VkDescriptorImageInfo> imageInfos;
    return _toHandles<SampledHandle>(
        _registerTexture(textures, imageInfos, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, _sampledTextures, 0, true, contiguous)
    );
}

std::optional<std::vector<ShaderResourceIndex>> DescriptorRegistry::_registerTexture(
    std::span<const Texture* const> textures,
    std::vector<VkDescriptorImageInfo>& imageInfos,
    VkDescriptorType descriptorType,
    IndexAllocator& indexAllocator,
    uint32_t binding,
    bool update,
    bool contiguous
) noexcept {
    imageInfos.reserve(textures.size());
    for (const auto& texture : textures) {
        auto imageInfoOpt = texture->descriptorImageInfoNoSampler();
        if (!imageInfoOpt) {
            return std::nullopt;
        }
        if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
            imageInfoOpt->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
        imageInfos.push_back(*imageInfoOpt);
    }

    std::vector<ShaderResourceIndex> indices;
    indices.reserve(imageInfos.size());
    if (contiguous) {
        auto first = indexAllocator.allocate(static_cast<ShaderResourceIndex>(imageInfos.size()));
        if (!first) {
            return std::nullopt;
        }
        for (ShaderResourceIndex i = 0; i < imageInfos.size(); ++i) {
            indices.push_back(first.value() + i);
        }
    } else {
        for (size_t i = 0; i < imageInfos.size(); ++i) {
            auto index = indexAllocator.allocate();
            if (!index) {
                for (ShaderResourceIndex allocated : indices) {
                    indexAllocator.release(allocated);
                }
                return std::nullopt;
            }
            indices.push_back(index.value());
        }
    }

    std::vector<VkWriteDescriptorSet> writes;
    writes.reserve(imageInfos.size());
    for (size_t i = 0; i < imageInfos.size(); ++i) {
        writes.push_back(
            VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = _descriptorSet,
            .dstBinding = binding,
            .dstArrayElement = indices[i],
            .descriptorCount = 1,
            .descriptorType = descriptorType,
            .pImageInfo = &imageInfos[i]
            }
        );
    }
    if (update) {
        vkUpdateDescriptorSets(
            _device.get().vkDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr
        );
    } else {
        _writes.insert(_writes.end(), writes.begin(), writes.end());
    }
    return indices;
}
