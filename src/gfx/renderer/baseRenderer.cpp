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

bool BaseRenderer::_bindBindlessDescriptorSet(vax::rhi::CommandBuffer& commandBuffer) noexcept {
    auto& pipelineManager = *_vkEngine.get().pipelineManager;
    VkPipelineLayout pipelineLayout = pipelineManager.pipelineLayout();
    auto bindlessDescriptorSet = pipelineManager.bindlessDescriptorSet();
    if (pipelineLayout == VK_NULL_HANDLE || bindlessDescriptorSet == VK_NULL_HANDLE) {
        _logger.error("Pipeline layout or descriptor sets are not created!");
        return false;
    }
    vkCmdBindDescriptorSets(
        commandBuffer.vkCommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        BINDLESS_SET_INDEX,
        1,
        &bindlessDescriptorSet,
        1,
        nullptr
    );
    return true;
}

bool BaseRenderer::load(
    std::shared_ptr<AssetsLibrary> assetsLibrary, uint32_t maxCommands, uint32_t maxInstances
) noexcept {
    _indirectDrawController->setup(maxCommands, _vkEngine.get().pipelineManager->descriptorRegistry());
    _sceneComposer =
        std::make_unique<SceneComposer>(std::move(assetsLibrary), _resourceManager.get().ssboManager(), maxInstances);
    return true;
}