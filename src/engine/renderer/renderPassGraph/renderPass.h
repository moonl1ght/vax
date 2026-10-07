#pragma once

#include "commandBuffer.h"
#include "device.h"
#include "pipeline.h"
#include "renderDestination.h"
#include "renderPassDescriptor.h"
#include "renderPassNode.h"
#include "renderSubpass.h"
#include "vaxMath.h"

namespace vax::engine {
class RenderPass : public RenderPassNode {
  public:
    RenderPass(
        std::string_view id,
        vax::vk::Device& device,
        vax::vk::PipelineManager& pipelineManager,
        vax::vk::DescriptorSetManager& descriptorSetManager,
        std::string_view debugName,
        std::weak_ptr<vax::vk::RenderDestination> renderDestination,
        std::weak_ptr<vax::vk::RenderPassDescriptor> renderDescriptor
    )
        : RenderPassNode(id)
        , _device(device)
        , _pipelineManager(pipelineManager)
        , _descriptorSetManager(descriptorSetManager)
        , _logger(vax::Logger(std::string(debugName)))
        , _debugName(debugName)
        , _renderDestination(std::move(renderDestination))
        , _renderDescriptor(std::move(renderDescriptor)) {
        if (auto renderDescriptorShared = _renderDescriptor.lock()) {
            _clearValues = renderDescriptorShared->colorClearValues;
            _clearValues.push_back(VkClearValue{.depthStencil = {0.0f, 0}});
        }
    };

    RenderPass(const RenderPass&) = delete;
    RenderPass(RenderPass&&) = default;
    RenderPass& operator=(const RenderPass&) = delete;
    RenderPass& operator=(RenderPass&&) = default;

    virtual ~RenderPass() = default;

    void runPass(RunPassInfo& runPassInfo) override;

    void setRenderArea(VkRect2D renderArea) { _renderArea = renderArea; }

    void addInputDescriptorSet(InputDescriptorSetInfo inputDescriptorSetInfo) {
        _inputDescriptorSetInfos.push_back(inputDescriptorSetInfo);
    }

    void updateInputDescriptorSetAt(size_t index, InputDescriptorSetInfo inputDescriptorSetInfo) {
        _inputDescriptorSetInfos[index] = inputDescriptorSetInfo;
    }

    const InputDescriptorSetInfo& getInputDescriptorSetAt(size_t index) const {
        return _inputDescriptorSetInfos[index];
    }

    InputDescriptorSetInfo& getInputDescriptorSetAt(size_t index) { return _inputDescriptorSetInfos[index]; }

    void setPipeline(vax::vk::PipelineName pipelineName) { _pipelineName = pipelineName; }

    void setPipelineLayout(vax::vk::PipelineLayoutName pipelineLayoutName) { _pipelineLayoutName = pipelineLayoutName; }

    void setDrawWork(std::function<void(RunPassInfo&, DrawContext&)> drawWork) { _drawWork = drawWork; }

    void setOutsideDrawWork(std::function<void(RunPassInfo&)> outsideDrawWork) { _outsideDrawWork = outsideDrawWork; }

    void setRenderToSwapchain(bool renderToSwapchain) { _renderToSwapchain = renderToSwapchain; }

    void setOutsideRenderPass(bool outsideRenderPass) { _outsideRenderPass = outsideRenderPass; }

    void setRenderOffset(vax::math::Position2DFloat offset) { _offset = offset; }

    void addSubpass(std::unique_ptr<RenderSubpass> subpass) { _subpasses.push_back(std::move(subpass)); }

    void setOutputImageIndex(size_t outputImageIndex) { _outputImageIndex = outputImageIndex; }

  protected:
    vax::Logger _logger;
    std::reference_wrapper<vax::vk::Device> _device;
    std::reference_wrapper<vax::vk::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::vk::DescriptorSetManager> _descriptorSetManager;

    std::weak_ptr<vax::vk::RenderDestination> _renderDestination;
    std::weak_ptr<vax::vk::RenderPassDescriptor> _renderDescriptor;

    std::vector<InputDescriptorSetInfo> _inputDescriptorSetInfos;
    std::vector<std::unique_ptr<RenderSubpass>> _subpasses;

    std::string _debugName;
    std::vector<VkClearValue> _clearValues;
    std::function<void(RunPassInfo&, DrawContext&)> _drawWork;
    std::function<void(RunPassInfo&)> _outsideDrawWork;
    VkRect2D _renderArea{};
    vax::vk::PipelineName _pipelineName = vax::vk::PipelineName::UNKNOWN;
    vax::vk::PipelineLayoutName _pipelineLayoutName = vax::vk::PipelineLayoutName::UNKNOWN;
    bool _renderToSwapchain = false;
    bool _outsideRenderPass = false;
    vax::math::Position2DFloat _offset = {0.0f, 0.0f};
    size_t _outputImageIndex = 0;

    template <typename Work> void _pass(RunPassInfo& runPassInfo, Work work) {
        auto renderDestinationShared = _renderDestination.lock();
        if (!renderDestinationShared) {
            _logger.error("Render destination not found!");
            return;
        }
        auto renderDescriptorShared = _renderDescriptor.lock();
        if (!renderDescriptorShared) {
            _logger.error("Render descriptor not found!");
            return;
        }
        auto& framebuffers = renderDestinationShared->framebuffers;
        VkRenderPassBeginInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = renderDescriptorShared->getVkRenderPass(),
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
                .objectHandle = reinterpret_cast<size_t>(renderDescriptorShared->getVkRenderPass()),
                .pObjectName = _debugName.data(),
            };
            vax::vk::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice, &nameInfo);

            VkDebugUtilsLabelEXT label{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pLabelName = _debugName.data(),
            };
            vax::vk::pfnCmdBeginDebugUtilsLabelEXT(runPassInfo.commandBuffer.vkCommandBuffer, &label);
        }
        vkCmdBeginRenderPass(runPassInfo.commandBuffer.vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        work();
        vkCmdEndRenderPass(runPassInfo.commandBuffer.vkCommandBuffer);
        if (!_debugName.empty()) {
            vax::vk::pfnCmdEndDebugUtilsLabelEXT(runPassInfo.commandBuffer.vkCommandBuffer);
        }
    }

    void _setViewportAndScissor(vax::vk::CommandBuffer& commandBuffer, VkExtent2D extent);
};
} // namespace vax::engine