#pragma once

#include "commandBuffer.h"
#include "frameProfiler.h"
#include "indirectDrawController.h"
#include "logger.h"
#include "renderPassGraphBuilder.h"
#include "renderTarget.h"
#include "vkEngine.h"

namespace vax::gfx {
class BaseRenderer {
  public:
    BaseRenderer(rhi::VulkanEngine& vkEngine)
        : _vkEngine(vkEngine) {
        _indirectDrawController = std::make_unique<IndirectDrawController>(*_vkEngine.get().device);
    };

    virtual ~BaseRenderer() {};

    BaseRenderer(const BaseRenderer& other) = delete;
    BaseRenderer& operator=(const BaseRenderer& other) = delete;
    BaseRenderer(BaseRenderer&& other) noexcept = delete;
    BaseRenderer& operator=(BaseRenderer&& other) noexcept = delete;

    void linkFrameProfiler(dev::FrameProfiler* frameProfiler) noexcept { _frameProfiler = frameProfiler; }

    void unlinkFrameProfiler(dev::FrameProfiler* frameProfiler) noexcept {
        if (_frameProfiler == frameProfiler) {
            _frameProfiler = nullptr;
        }
    }

  protected:
    dev::Logger _logger = dev::Logger("BaseRenderer");

    std::reference_wrapper<rhi::VulkanEngine> _vkEngine;

    std::unique_ptr<IndirectDrawController> _indirectDrawController;
    std::unique_ptr<RenderTarget> _renderTarget;
    std::unique_ptr<RenderPassGraphBuilder> _renderPassGraphBuilder;

    dev::FrameProfiler* _frameProfiler = nullptr;

    std::vector<uint32_t> _outputImageIndices;

    uint32_t _currentFrame = 0;

    void _setViewportAndScissor(vax::rhi::CommandBuffer& commandBuffer, VkExtent2D extent);

    void _waitForFence();

    bool _bindBindlessDescriptorSet(vax::rhi::CommandBuffer& commandBuffer) noexcept;
};
} // namespace vax::gfx