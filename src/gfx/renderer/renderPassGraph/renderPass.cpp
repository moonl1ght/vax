#include "renderPass.h"

using namespace vax::gfx;

void RenderPass::runPass(RunPassInfo& runPassInfo) {
    _setViewportAndScissor(runPassInfo.commandBuffer, _renderArea.extent);
    _pass(runPassInfo, [&]() {
        DrawContext drawContext{
            .commandBuffer = runPassInfo.commandBuffer,
            .currentFrame = runPassInfo.frameIndex,
        };
        for (const auto& step : _steps) {
            step->run(runPassInfo, drawContext);
        }
    });
}

void RenderPass::_setViewportAndScissor(vax::rhi::CommandBuffer& commandBuffer, VkExtent2D extent) {
    VkViewport viewport{
        .x = _offset.x,
        .y = static_cast<float>(extent.height),
        .width = static_cast<float>(extent.width),
        .height = -static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(commandBuffer.vkCommandBuffer, 0, 1, &viewport);

    VkRect2D scissor{.offset = {static_cast<int32_t>(_offset.x), static_cast<int32_t>(_offset.y)}, .extent = extent};
    vkCmdSetScissor(commandBuffer.vkCommandBuffer, 0, 1, &scissor);
}