#pragma once

#include <vulkan/vulkan_core.h>

namespace vax::vk {

struct PipelineV2 final {
  public:
    VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
    VkPipeline vkPipeline = VK_NULL_HANDLE;
};

} // namespace vax::vk
