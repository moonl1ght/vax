#pragma once

#include "device.h"
#include "drawContext.h"
#include "pipelineManager.h"
#include "renderPassNode.h"
#include <string>

namespace vax::gfx {
class RenderSubpass final {
  public:
    RenderSubpass(const std::string& name, vax::rhi::Device& device, vax::rhi::PipelineManager& pipelineManager)
        : _device(device)
        , _pipelineManager(pipelineManager) {}

    ~RenderSubpass() = default;

    RenderSubpass(const RenderSubpass& other) = delete;
    RenderSubpass(RenderSubpass&& other) noexcept = default;
    RenderSubpass& operator=(const RenderSubpass& other) = delete;
    RenderSubpass& operator=(RenderSubpass&& other) noexcept = default;

    void setDrawWork(std::function<void(RenderSubpass*, RenderPassNode::RunPassInfo&, DrawContext&)> drawWork) {
        _drawWork = drawWork;
    }

    void run(RenderPassNode::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        if (_drawWork) {
            auto pipeline = _pipelineManager.get().getPipeline(_pipelineName);
            if (!pipeline)
                return;
            if (!runPassInfo.commandBuffer.bindPipeline(&pipeline.value(), VK_PIPELINE_BIND_POINT_GRAPHICS))
                return;
            DrawContext drawContext{
                .commandBuffer = runPassInfo.commandBuffer,
                .currentFrame = runPassInfo.frameIndex,
            };
            _drawWork(this, runPassInfo, drawContext);
        }
    }

    void setRenderArea(VkRect2D renderArea) { _renderArea = renderArea; }

    VkRect2D getRenderArea() const { return _renderArea; }

    void setSwapchainExtent(VkExtent2D swapchainExtent) { _swapchainExtent = swapchainExtent; }

    VkExtent2D getSwapchainExtent() const { return _swapchainExtent; }

    void setPipeline(std::string pipelineName) { _pipelineName = pipelineName; }

  private:
    std::string _name;
    std::reference_wrapper<vax::rhi::Device> _device;
    std::reference_wrapper<vax::rhi::PipelineManager> _pipelineManager;
    VkExtent2D _swapchainExtent{};
    VkRect2D _renderArea{};
    std::string _pipelineName;

    std::function<void(RenderSubpass*, RenderPassNode::RunPassInfo&, DrawContext&)> _drawWork;
};
} // namespace vax::gfx