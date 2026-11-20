#pragma once

#include "commandBuffer.h"
#include "logger.h"
#include "swapchain.h"
#include "uiEngine.h"
#include "vkEngine.h"

namespace vax::gfx {
class BaseRenderer {
  public:
    BaseRenderer(rhi::Engine& vkEngine, gfx::UIEngine& uiEngine)
        : _vkEngine(vkEngine)
        , _uiEngine(uiEngine) {};

    virtual ~BaseRenderer() {};

    BaseRenderer(const BaseRenderer& other) = delete;
    BaseRenderer& operator=(const BaseRenderer& other) = delete;
    BaseRenderer(BaseRenderer&& other) noexcept = delete;
    BaseRenderer& operator=(BaseRenderer&& other) noexcept = delete;

  protected:
    struct SwapchainResult {
        uint32_t imageIndex;
        bool shouldRecreateSwapchain;
        bool hasError;
    };
    dev::Logger _logger = dev::Logger("BaseRenderer");

    std::reference_wrapper<rhi::Engine> _vkEngine;
    std::reference_wrapper<gfx::UIEngine> _uiEngine;

    std::vector<uint32_t> _outputImageIndices;

    uint32_t _currentFrame = 0;

    void _setViewportAndScissor(vax::rhi::CommandBuffer& commandBuffer, VkExtent2D extent);

    void _waitForFence();

    SwapchainResult _acquireSwapchainImage(size_t windowIndex, std::string semaphoreName);

    rhi::Swapchain* _getSwapchain(size_t index);
};
} // namespace vax::gfx