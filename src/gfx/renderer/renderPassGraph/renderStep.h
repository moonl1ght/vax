#pragma once

#include "device.h"
#include "drawContext.h"
#include "pipelineManager.h"
#include "renderPassNode.h"
#include <string>

namespace vax::gfx {
class RenderStep final {
  public:
    using DrawWork = std::function<void(RenderPassNode::RunPassInfo&, DrawContext&)>;

    RenderStep(vax::rhi::Device& device, vax::rhi::PipelineManager& pipelineManager)
        : _device(device)
        , _pipelineManager(pipelineManager) {}

    ~RenderStep() = default;

    RenderStep(const RenderStep& other) = delete;
    RenderStep(RenderStep&& other) noexcept = default;
    RenderStep& operator=(const RenderStep& other) = delete;
    RenderStep& operator=(RenderStep&& other) noexcept = default;

    void setDrawWork(DrawWork drawWork) { _drawWork = drawWork; }

    void setPipelineName(const std::string& pipelineName) { _pipelineName = pipelineName; }

    void run(RenderPassNode::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        if (_drawWork) {
            if (_pipelineName) {
                auto pipeline = _pipelineManager.get().getPipeline(_pipelineName.value());
                if (!pipeline)
                    return;
                if (!runPassInfo.commandBuffer.bindPipeline(&pipeline.value(), VK_PIPELINE_BIND_POINT_GRAPHICS))
                    return;
            }
            DrawContext drawContext{
                .commandBuffer = runPassInfo.commandBuffer,
                .currentFrame = runPassInfo.frameIndex,
            };
            _drawWork(runPassInfo, drawContext);
        }
    }

  private:
    std::optional<std::string> _pipelineName;
    std::reference_wrapper<vax::rhi::Device> _device;
    std::reference_wrapper<vax::rhi::PipelineManager> _pipelineManager;

    DrawWork _drawWork;
};
} // namespace vax::gfx