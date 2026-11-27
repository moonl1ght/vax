#pragma once

#include "commandBuffer.h"
#include "frameProfiler.h"
#include "indirectDrawController.h"
#include "logger.h"
#include "renderPassGraphBuilder.h"
#include "renderTarget.h"
#include "sceneComposer.h"
#include "vkEngine.h"
#include <cstdint>

namespace vax::gfx {
class BaseRenderer {
  public:
    BaseRenderer(rhi::VulkanEngine& vkEngine, vax::rhi::ResourceManager& resourceManager)
        : _vkEngine(vkEngine)
        , _resourceManager(resourceManager)
        , _uniformBufferStride(
              static_cast<uint32_t>(vkEngine.device->minUniformBufferOffsetAlignment<UniformBufferObject>())
          ) {
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

    bool load(std::shared_ptr<AssetsLibrary> assetsLibrary, uint32_t maxCommands, uint32_t maxInstances) noexcept;

  protected:
    dev::Logger _logger = dev::Logger("BaseRenderer");

    const uint32_t _uniformBufferStride = 0;

    std::reference_wrapper<rhi::VulkanEngine> _vkEngine;
    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;

    std::unique_ptr<IndirectDrawController> _indirectDrawController;
    std::unique_ptr<RenderTarget> _renderTarget;
    std::unique_ptr<RenderPassGraphBuilder> _renderPassGraphBuilder;
    std::unique_ptr<SceneComposer> _sceneComposer;

    dev::FrameProfiler* _frameProfiler = nullptr;

    std::vector<uint32_t> _outputImageIndices;

    uint32_t _currentFrame = 0;

    void _setViewportAndScissor(vax::rhi::CommandBuffer& commandBuffer, VkExtent2D extent);

    void _waitForFence();

    bool _bindBindlessDescriptorSet(vax::rhi::CommandBuffer& commandBuffer) noexcept;
};
} // namespace vax::gfx