#include "baseRenderer.h"

using namespace vax::gfx;

void BaseRenderer::_setViewportAndScissor(vax::rhi::CommandBuffer& commandBuffer, VkExtent2D extent) {
    VkViewport viewport{
        .x = 0.0f,
        .y = static_cast<float>(extent.height),
        .width = static_cast<float>(extent.width),
        .height = -static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(commandBuffer.vkCommandBuffer, 0, 1, &viewport);

    VkRect2D scissor{.offset = {0, 0}, .extent = extent};
    vkCmdSetScissor(commandBuffer.vkCommandBuffer, 0, 1, &scissor);
}

void BaseRenderer::_waitForFence() {
    vkWaitForFences(
        _vkEngine.get().device->vkDevice(),
        1,
        &_vkEngine.get().syncObjectsManager->getInFlightFences()[_currentFrame],
        VK_TRUE,
        UINT64_MAX
    );
}