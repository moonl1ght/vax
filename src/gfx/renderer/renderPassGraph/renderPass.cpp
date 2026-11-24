#include "renderPass.h"

using namespace vax::gfx;

void RenderPass::runPass(RunPassInfo& runPassInfo) {
    _setViewportAndScissor(runPassInfo.commandBuffer, _renderArea.extent);
    _pass(runPassInfo, [&]() {
        if (_pipelineName) {
            auto pipeline = _pipelineManager.get().getPipeline(_pipelineName.value());
            if (!pipeline) {
                _logger.error("Failed to get pipeline!");
                return;
            }

            if (!runPassInfo.commandBuffer.bindPipeline(&pipeline.value(), VK_PIPELINE_BIND_POINT_GRAPHICS))
                return;
        }
        DrawContext drawContext{
            .commandBuffer = runPassInfo.commandBuffer,
            .currentFrame = runPassInfo.frameIndex,
        };
        if (_drawWork) {
            _drawWork(runPassInfo, drawContext);
        }
        for (const auto& subpass : _subpasses) {
            subpass->run(runPassInfo, drawContext);
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