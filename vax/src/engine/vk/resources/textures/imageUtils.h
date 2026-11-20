#pragma once

#include "vulkanCommon.h"
#include <cstdint>
#include <optional>
#include <utility>

namespace vax::vk {
class CommandBuffer;
class Engine;
}; // namespace vax::vk

namespace vax::vk {

// TODO: Move to textureTaskScheduler
void copyImageToImage(
    VkCommandBuffer commandBuffer, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize
);

std::optional<VkImageView> createImageView(
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectMask,
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D,
    uint32_t layerCount = 1,
    uint32_t levelCount = 1
);

std::optional<std::pair<VkImage, VmaAllocation>> createImage(
    VmaAllocator allocator,
    VkExtent3D extent,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    uint32_t layers = 1,
    uint32_t mipLevels = 1,
    VkImageCreateFlags flags = 0
);
}; // namespace vax::vk