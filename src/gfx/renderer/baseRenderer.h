#pragma once

#include "commandBuffer.h"
#include "logger.h"
#include "swapchain.h"
#include "vkEngine.h"

namespace vax::gfx {
class BaseRenderer {
  public:
    BaseRenderer(rhi::VulkanEngine& vkEngine)
        : _vkEngine(vkEngine) {};

    virtual ~BaseRenderer() {};

    BaseRenderer(const BaseRenderer& other) = delete;
    BaseRenderer& operator=(const BaseRenderer& other) = delete;
    BaseRenderer(BaseRenderer&& other) noexcept = delete;
    BaseRenderer& operator=(BaseRenderer&& other) noexcept = delete;

  protected:
    dev::Logger _logger = dev::Logger("BaseRenderer");

    std::reference_wrapper<rhi::VulkanEngine> _vkEngine;

    std::vector<uint32_t> _outputImageIndices;

    uint32_t _currentFrame = 0;

    void _setViewportAndScissor(vax::rhi::CommandBuffer& commandBuffer, VkExtent2D extent);

    void _waitForFence();
};
} // namespace vax::gfx