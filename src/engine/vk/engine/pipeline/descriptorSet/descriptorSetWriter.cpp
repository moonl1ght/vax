#include "descriptorSetWriter.h"
#include "shaderUniforms.h"
#include "vkUtils.h"

using namespace vax::vk;
using namespace vax;

bool DescriptorSetWriter::writeTexture(
    const Texture& texture, uint32_t binding, uint32_t arrayElement, bool withSampler
) {
    auto imageInfoOpt = withSampler ? texture.descriptorImageInfoWithSampler() : texture.descriptorImageInfoNoSampler();
    if (!imageInfoOpt) {
        return false;
    }

    VkDescriptorImageInfo& imageInfo = _imageInfos.emplace_back(imageInfoOpt.value());

    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSet,
        .dstBinding = binding,
        .dstArrayElement = arrayElement,
        .descriptorCount = 1,
        .descriptorType = withSampler ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &imageInfo
    };

    _writes.push_back(write);
    return true;
}

bool DescriptorSetWriter::writeStorageTexture(const Texture& texture, uint32_t binding, uint32_t arrayElement) {
    auto imageInfoOpt = texture.descriptorImageInfoNoSampler();
    if (!imageInfoOpt) {
        return false;
    }
    VkDescriptorImageInfo& imageInfo = _imageInfos.emplace_back(imageInfoOpt.value());
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    _writes.push_back(
        VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSet,
        .dstBinding = binding,
        .dstArrayElement = arrayElement,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &imageInfo,
        }
    );
    return true;
}

bool DescriptorSetWriter::writeTextures(
    const std::vector<const Texture*>& textures, uint32_t binding, bool withSampler
) {
    std::vector<VkDescriptorImageInfo> imageInfos;
    imageInfos.reserve(textures.size());
    for (const auto& texture : textures) {
        auto imageInfoOpt =
            withSampler ? texture->descriptorImageInfoWithSampler() : texture->descriptorImageInfoNoSampler();
        if (!imageInfoOpt) {
            return false;
        }
        imageInfos.push_back(*imageInfoOpt);
    }
    auto& imageInfosSaved = _imageInfosArray.emplace_back(imageInfos);

    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSet,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = static_cast<uint32_t>(imageInfosSaved.size()),
        .descriptorType = withSampler ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = imageInfosSaved.data()
    };

    _writes.push_back(write);
    return true;
}

void DescriptorSetWriter::writeSampler(const Sampler& sampler, uint32_t binding, uint32_t arrayElement) {
    VkDescriptorImageInfo& samplerInfo = _imageInfos.emplace_back(
        VkDescriptorImageInfo{
        .sampler = sampler.vkSampler,
        }
    );

    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptorSet,
        .dstBinding = binding,
        .dstArrayElement = arrayElement,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &samplerInfo
    };

    _writes.push_back(write);
}

VkDescriptorSet DescriptorSetWriter::update() {
    vkUpdateDescriptorSets(_device.get().vkDevice, static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);
    return _descriptorSet;
}

void DescriptorSetWriter::clear() {
    _writes.clear();
    _bufferInfos.clear();
    _imageInfos.clear();
    _imageInfosArray.clear();
}

void DescriptorSetWriter::bind(
    VkCommandBuffer commandBuffer,
    VkPipelineLayout pipelineLayout,
    uint32_t setIndex,
    VkPipelineBindPoint bindPoint,
    uint32_t dynamicOffsetCount,
    const uint32_t* pDynamicOffsets
) {
    vkCmdBindDescriptorSets(
        commandBuffer, bindPoint, pipelineLayout, setIndex, 1, &_descriptorSet, dynamicOffsetCount, pDynamicOffsets
    );
}