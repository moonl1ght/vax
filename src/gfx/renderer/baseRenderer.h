#pragma once

#include "commandBuffer.h"
#include "logger.h"
#include "swapchain.h"
#include "uiEngine.h"
#include "vkEngine.h"

namespace vax::gfx {
class BaseRenderer {
  public:
    BaseRenderer(rhi::VulkanEngine& vkEngine, editor::UIEngine& uiEngine)
        : _vkEngine(vkEngine)
        , _uiEngine(uiEngine) {};

    virtual ~BaseRenderer() {};

    BaseRenderer(const BaseRenderer& other) = delete;
    BaseRenderer& operator=(const BaseRenderer& other) = delete;
    BaseRenderer(BaseRenderer&& other) noexcept = delete;
    BaseRenderer& operator=(BaseRenderer&& other) noexcept = delete;

  protected:
    dev::Logger _logger = dev::Logger("BaseRenderer");

    std::reference_wrapper<rhi::VulkanEngine> _vkEngine;
    std::reference_wrapper<editor::UIEngine> _uiEngine;

    std::vector<uint32_t> _outputImageIndices;

    uint32_t _currentFrame = 0;

    void _setViewportAndScissor(vax::rhi::CommandBuffer& commandBuffer, VkExtent2D extent);

    void _waitForFence();
};
} // namespace vax::gfx