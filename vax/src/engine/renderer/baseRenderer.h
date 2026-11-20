#pragma once

#include "commandBuffer.h"
#include "logger.h"
#include "swapchain.h"
#include "uiEngine.h"
#include "vkEngine.h"

namespace vax::engine {
class BaseRenderer {
  public:
    BaseRenderer(vax::vk::Engine& vkEngine, vax::ui::UIEngine& uiEngine)
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
    vax::Logger _logger = vax::Logger("BaseRenderer");

    std::reference_wrapper<vax::vk::Engine> _vkEngine;
    std::reference_wrapper<vax::ui::UIEngine> _uiEngine;

    std::vector<uint32_t> _outputImageIndices;

    uint32_t _currentFrame = 0;

    void _setViewportAndScissor(vax::vk::CommandBuffer& commandBuffer, VkExtent2D extent);

    void _waitForFence();

    SwapchainResult _acquireSwapchainImage(size_t windowIndex, std::string semaphoreName);

    vax::vk::Swapchain* _getSwapchain(size_t index);
};
} // namespace vax::engine