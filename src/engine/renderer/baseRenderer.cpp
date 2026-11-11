#include "baseRenderer.h"

using namespace vax::engine;

void BaseRenderer::_setViewportAndScissor(vax::vk::CommandBuffer& commandBuffer, VkExtent2D extent) {
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

vax::vk::Swapchain* BaseRenderer::_getSwapchain(size_t index) {
    if (index >= vax::WindowController::maxWindows) {
        _logger.error("Invalid window index");
        return nullptr;
    }
    return _vkEngine.get().getWindowController().getWindow(index)->getSwapchain();
}

BaseRenderer::SwapchainResult BaseRenderer::_acquireSwapchainImage(size_t windowIndex, std::string semaphoreName) {
    uint32_t imageIndex = UINT32_MAX;
    bool hasError = false;
    bool shouldRecreateSwapchain = false;
    VkResult result = vkAcquireNextImageKHR(
        _vkEngine.get().device->vkDevice(),
        _vkEngine.get().getWindowController().getWindow(windowIndex)->getSwapchain()->swapchain,
        UINT64_MAX,
        _vkEngine.get().syncObjectsManager->getImageAvailableSemaphores(semaphoreName)[_currentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        _logger.warning("Failed to acquire swap chain image");
        hasError = true;
    }
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        shouldRecreateSwapchain = true;
    }
    return BaseRenderer::SwapchainResult{
        .imageIndex = imageIndex,
        .shouldRecreateSwapchain = false,
        .hasError = false
    };
}