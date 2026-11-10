#pragma once

#include "buffer.h"
#include "device.h"
#include "sampler.h"
#include "texture.h"
#include <deque>

namespace vax::vk {
struct DescriptorSetWriter final {
  public:
    explicit DescriptorSetWriter(const vax::vk::Device& device, VkDescriptorSet descriptorSet)
        : _device(device)
        , _descriptorSet(descriptorSet) {
        _writes.reserve(100);
    }

    ~DescriptorSetWriter() {}

    DescriptorSetWriter(const DescriptorSetWriter&) = delete;
    DescriptorSetWriter& operator=(const DescriptorSetWriter&) = delete;
    DescriptorSetWriter(DescriptorSetWriter&& other) noexcept = default;
    DescriptorSetWriter& operator=(DescriptorSetWriter&& other) noexcept = default;

    template <typename T>
    void writeBuffer(
        const Buffer<T>& buffer,
        uint32_t binding,
        uint32_t offset,
        VkDescriptorType descriptorType,
        uint32_t arrayElement = 0
    ) {
        VkDescriptorBufferInfo& bufferInfo = _bufferInfos.emplace_back(
            VkDescriptorBufferInfo{.buffer = buffer.vkBuffer(), .offset = offset, .range = buffer.size()}
        );

        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = _descriptorSet,
            .dstBinding = binding,
            .dstArrayElement = arrayElement,
            .descriptorCount = 1,
            .descriptorType = descriptorType,
            .pBufferInfo = &bufferInfo
        };

        _writes.push_back(write);
    }

    bool writeTexture(const Texture& texture, uint32_t binding, uint32_t arrayElement = 0, bool withSampler = false);

    bool writeStorageTexture(const Texture& texture, uint32_t binding, uint32_t arrayElement = 0);

    bool writeTextures(const std::vector<const Texture*>& textures, uint32_t binding, bool withSampler = false);

    void writeSampler(const Sampler& sampler, uint32_t binding, uint32_t arrayElement = 0);

    VkDescriptorSet getDescriptorSet() const { return _descriptorSet; }

    VkDescriptorSet update();

    void clear();

    void bind(
        VkCommandBuffer commandBuffer,
        VkPipelineLayout pipelineLayout,
        uint32_t setIndex,
        VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        uint32_t dynamicOffsetCount = 0,
        const uint32_t* pDynamicOffsets = nullptr
    );

  private:
    std::reference_wrapper<const vax::vk::Device> _device;
    VkDescriptorSet _descriptorSet;
    std::vector<VkWriteDescriptorSet> _writes;
    std::deque<VkDescriptorBufferInfo> _bufferInfos;
    std::deque<VkDescriptorImageInfo> _imageInfos;
    std::deque<std::vector<VkDescriptorImageInfo>> _imageInfosArray;
};
} // namespace vax::vk