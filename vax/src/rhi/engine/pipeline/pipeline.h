#pragma once

#include <hashUtils.h>
#include <vulkan/vulkan_core.h>

namespace vax::vk {

struct Pipeline final {
    VkPipeline vkPipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSetLayout = VK_NULL_HANDLE;
};

} // namespace vax::vk