#pragma once

#include "commandBuffer.h"
#include "device.h"
#include "pipeline.h"
#include "renderDestination.h"
#include "renderPassDescriptor.h"
#include "renderPassNode.h"
#include "renderStep.h"
#include "vaxMath.h"

namespace vax::gfx {
class RenderPass : public RenderPassNode {
  public:
    RenderPass(
        std::string_view id,
        vax::rhi::Device& device,
        std::string_view debugName,
        vax::rhi::RenderDestination* renderDestination,
        vax::rhi::RenderPassDescriptor* renderDescriptor
    )
        : RenderPassNode(id)
        , _device(device)
        , _logger(dev::Logger(std::string(debugName)))
        , _debugName(debugName)
        , _renderDestination(renderDestination)
        , _renderDescriptor(renderDescriptor) {
        _clearValues = renderDescriptor->colorClearValues;
        _clearValues.push_back(VkClearValue{.depthStencil = {0.0f, 0}});
    };

    RenderPass(const RenderPass&) = delete;
    RenderPass(RenderPass&&) = default;
    RenderPass& operator=(const RenderPass&) = delete;
    RenderPass& operator=(RenderPass&&) = default;

    virtual ~RenderPass() = default;

    void runPass(RunPassInfo& runPassInfo) override;

    void setRenderArea(VkRect2D renderArea) { _renderArea = renderArea; }

    void setRenderToSwapchain(bool renderToSwapchain) { _renderToSwapchain = renderToSwapchain; }

    void setRenderOffset(vax::math::Position2DFloat offset) { _offset = offset; }

    void addStep(std::shared_ptr<RenderStep> step) { _steps.push_back(std::move(step)); }

    void setOutputImageIndex(size_t outputImageIndex) { _outputImageIndex = outputImageIndex; }

  protected:
    dev::Logger _logger;
    std::reference_wrapper<vax::rhi::Device> _device;

    vax::rhi::RenderDestination* _renderDestination;
    vax::rhi::RenderPassDescriptor* _renderDescriptor;

    std::vector<std::shared_ptr<RenderStep>> _steps;

    std::string _debugName;
    std::vector<VkClearValue> _clearValues;
    VkRect2D _renderArea{};
    bool _renderToSwapchain = false;
    vax::math::Position2DFloat _offset = {0.0f, 0.0f};
    size_t _outputImageIndex = 0;

    template <typename Work> void _pass(RunPassInfo& runPassInfo, Work work) {
        auto& framebuffers = _renderDestination->framebuffers;
        VkRenderPassBeginInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = _renderDescriptor->getVkRenderPass(),
            .framebuffer =
                framebuffers[_renderToSwapchain ? runPassInfo.imageIndices[_outputImageIndex] : runPassInfo.frameIndex],
            .renderArea = _renderArea,
            .clearValueCount = static_cast<uint32_t>(_clearValues.size()),
            .pClearValues = _clearValues.data(),
        };
        if (!_debugName.empty()) {
            VkDebugUtilsObjectNameInfoEXT nameInfo{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .objectType = VK_OBJECT_TYPE_RENDER_PASS,
                .objectHandle = reinterpret_cast<size_t>(_renderDescriptor->getVkRenderPass()),
                .pObjectName = _debugName.data(),
            };
            vax::rhi::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice(), &nameInfo);

            VkDebugUtilsLabelEXT label{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pLabelName = _debugName.data(),
            };
            vax::rhi::pfnCmdBeginDebugUtilsLabelEXT(runPassInfo.commandBuffer.vkCommandBuffer, &label);
        }
        vkCmdBeginRenderPass(runPassInfo.commandBuffer.vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        work();
        vkCmdEndRenderPass(runPassInfo.commandBuffer.vkCommandBuffer);
        if (!_debugName.empty()) {
            vax::rhi::pfnCmdEndDebugUtilsLabelEXT(runPassInfo.commandBuffer.vkCommandBuffer);
        }
    }

    void _setViewportAndScissor(vax::rhi::CommandBuffer& commandBuffer, VkExtent2D extent);
};
} // namespace vax::gfx