#pragma once

#include "descriptorSetManager.h"
#include "device.h"
#include "drawContext.h"
#include "pipelineManager.h"
#include "renderPassNode.h"
#include <string>

namespace vax::engine {
class RenderSubpass final {
  public:
    RenderSubpass(
        const std::string& name,
        vax::vk::Device& device,
        vax::vk::PipelineManager& pipelineManager,
        vax::vk::DescriptorSetManager& descriptorSetManager
    )
        : _device(device)
        , _pipelineManager(pipelineManager)
        , _descriptorSetManager(descriptorSetManager) {}

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
            if (!runPassInfo.commandBuffer.bindPipeline(pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS))
                return;
            DrawContext drawContext{
                .commandBuffer = runPassInfo.commandBuffer,
                .pipelineLayout = pipeline->vkPipelineLayout,
                .currentFrame = runPassInfo.frameIndex,
            };
            _drawWork(this, runPassInfo, drawContext);
        }
    }

    void setRenderArea(VkRect2D renderArea) { _renderArea = renderArea; }

    VkRect2D getRenderArea() const { return _renderArea; }

    void setSwapchainExtent(VkExtent2D swapchainExtent) { _swapchainExtent = swapchainExtent; }

    VkExtent2D getSwapchainExtent() const { return _swapchainExtent; }

    void setPipeline(vax::vk::PipelineName pipelineName) { _pipelineName = pipelineName; }

  private:
    std::string _name;
    std::reference_wrapper<vax::vk::Device> _device;
    std::reference_wrapper<vax::vk::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::vk::DescriptorSetManager> _descriptorSetManager;
    VkExtent2D _swapchainExtent{};
    VkRect2D _renderArea{};
    vax::vk::PipelineName _pipelineName = vax::vk::PipelineName::UNKNOWN;

    std::function<void(RenderSubpass*, RenderPassNode::RunPassInfo&, DrawContext&)> _drawWork;
};
} // namespace vax::engine