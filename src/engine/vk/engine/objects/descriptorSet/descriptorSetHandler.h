#pragma once

#include "vulkan/vulkan_core.h"

namespace vax::vk {
struct DescriptorSetHandler final {
  public:
    explicit DescriptorSetHandler(VkDescriptorSet descriptorSet)
        : _descriptorSet(descriptorSet) {}

    ~DescriptorSetHandler() {}

    void bind(
        VkCommandBuffer commandBuffer,
        VkPipelineLayout pipelineLayout,
        uint32_t setIndex,
        VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        uint32_t dynamicOffsetCount = 0,
        const uint32_t* pDynamicOffsets = nullptr
    ) noexcept {
        if (!_descriptorSet) {
            return;
        }
        vkCmdBindDescriptorSets(
            commandBuffer, bindPoint, pipelineLayout, setIndex, 1, &_descriptorSet, dynamicOffsetCount, pDynamicOffsets
        );
    }

  private:
    VkDescriptorSet _descriptorSet;
};
} // namespace vax::vk