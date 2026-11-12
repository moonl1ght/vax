#pragma once

#include <descriptorSetHandler.h>
#include <descriptorSetWriter.h>
#include <hashUtils.h>
#include <vulkan/vulkan_core.h>

namespace vax::vk {

struct PipelineV2 final {
    VkPipeline vkPipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSetLayout = VK_NULL_HANDLE;
};

} // namespace vax::vk