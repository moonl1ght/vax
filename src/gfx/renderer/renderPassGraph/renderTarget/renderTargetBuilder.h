#pragma once

#include "commandManager.h"
#include "descriptorRegistry.h"
#include "device.h"
#include "logger.h"
#include "queueManager.h"
#include "swapchain.h"
#include <renderTarget.h>

namespace vax::gfx {

class RenderTargetBuilder final {
  public:
    explicit RenderTargetBuilder(
        const rhi::Device& device,
        rhi::DescriptorRegistry& descriptorRegistry,
        rhi::CommandManager& commandManager,
        rhi::QueueManager& queueManager
    ) noexcept
        : _device(device)
        , _descriptorRegistry(descriptorRegistry)
        , _commandManager(commandManager)
        , _queueManager(queueManager) {}

    ~RenderTargetBuilder() = default;

    RenderTargetBuilder(const RenderTargetBuilder&) = delete;
    RenderTargetBuilder& operator=(const RenderTargetBuilder&) = delete;
    RenderTargetBuilder(RenderTargetBuilder&&) noexcept = delete;
    RenderTargetBuilder& operator=(RenderTargetBuilder&&) noexcept = delete;

    std::unique_ptr<RenderTarget>
    buildForEditor(rhi::Swapchain& swapchain, rhi::Swapchain* agentCameraSwapchain = nullptr);

    std::unique_ptr<RenderTarget> buildForSimulation(VkFormat imageFormat, VkExtent2D extent);

  private:
    dev::Logger _logger = dev::Logger("RenderTargetBuilder");

    std::reference_wrapper<const rhi::Device> _device;
    std::reference_wrapper<rhi::DescriptorRegistry> _descriptorRegistry;
    std::reference_wrapper<rhi::CommandManager> _commandManager;
    std::reference_wrapper<rhi::QueueManager> _queueManager;
};

} // namespace vax::gfx
