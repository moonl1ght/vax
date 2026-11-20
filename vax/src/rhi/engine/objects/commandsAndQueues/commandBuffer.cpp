#include "commandBuffer.h"
#include "vkUtils.h"

using namespace vax::vk;

bool vax::vk::CommandBuffer::begin() {
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    auto result = VK_CHECK(vkBeginCommandBuffer(vkCommandBuffer, &beginInfo));
    if (result) {
        _isBegun = true;
        return true;
    } else {
        _logger.error("Failed to begin command buffer!");
        return false;
    }
}

bool vax::vk::CommandBuffer::end() {
    auto result = VK_CHECK(vkEndCommandBuffer(vkCommandBuffer));
    if (result) {
        _isBegun = false;
        return true;
    } else {
        _logger.error("Failed to end command buffer!");
        return false;
    }
}

void vax::vk::CommandBuffer::submitAndWait(VkQueue queue) {
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &vkCommandBuffer,
    };
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}

void vax::vk::CommandBuffer::reset() { vkResetCommandBuffer(vkCommandBuffer, 0); }

bool vax::vk::CommandBuffer::bindPipeline(const Pipeline* pipeline, VkPipelineBindPoint bindPoint) {
    if (pipeline == nullptr) {
        _logger.error("Failed to bind pipeline!");
        return false;
    }
    vkCmdBindPipeline(vkCommandBuffer, bindPoint, pipeline->vkPipeline);
    return true;
}