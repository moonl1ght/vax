#include "textureTaskScheduler.h"

using namespace vax::vk;

void TextureTaskSchedulerInline::transitionTextureLayout(
    Texture& texture, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask
) {
    VkImageMemoryBarrier2 imageBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = texture.image(),
        .subresourceRange = {
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        }
    };

    VkDependencyInfo depInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &imageBarrier
    };

    vkCmdPipelineBarrier2(_commandBuffer.get().vkCommandBuffer, &depInfo);
}

void TextureTaskScheduler::transitionTextureLayoutAndSubmit(
    VkQueue submitQueue,
    Texture& texture,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkImageAspectFlags aspectMask
) {
    auto commandBuffer = _commandManager.get().createSingleTimeCommandBuffer();
    auto taskSchedulerInline = TextureTaskSchedulerInline(_device.get(), commandBuffer);
    commandBuffer.begin();
    taskSchedulerInline.transitionTextureLayout(texture, oldLayout, newLayout, aspectMask);
    commandBuffer.end();
    commandBuffer.submitAndWait(submitQueue);
}

void TextureTaskSchedulerInline::copyBufferToTexture(vax::vk::AnyBuffer& buffer, Texture& texture) {
    VkBufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
        .imageOffset = {0, 0, 0},
        .imageExtent = {texture.width(), texture.height(), 1}
    };

    copyBufferToTexture(buffer, texture, {region});
}

void TextureTaskSchedulerInline::copyBufferToTexture(
    vax::vk::AnyBuffer& buffer, Texture& texture, const std::vector<VkBufferImageCopy>& copyRegions
) {
    vkCmdCopyBufferToImage(
        _commandBuffer.get().vkCommandBuffer,
        buffer.vkBuffer(),
        texture.image(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<uint32_t>(copyRegions.size()),
        copyRegions.data()
    );
}

void TextureTaskScheduler::copyBufferToTextureAndSubmit(
    VkQueue submitQueue, vax::vk::AnyBuffer& buffer, Texture& texture
) {
    auto commandBuffer = _commandManager.get().createSingleTimeCommandBuffer();
    auto taskSchedulerInline = TextureTaskSchedulerInline(_device.get(), commandBuffer);
    commandBuffer.begin();
    taskSchedulerInline.copyBufferToTexture(buffer, texture);
    commandBuffer.end();
    commandBuffer.submitAndWait(submitQueue);
}