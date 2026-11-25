#pragma once

#include "commandManager.h"
#include "descriptorRegistry.h"
#include "device.h"
#include "logger.h"
#include "renderDestination.h"
#include "renderPassDescriptor.h"
#include "swapchain.h"
#include <optional>
#include "textureFactory.h"
#include "gpuQueue.h"

namespace vax::rhi {
class RenderDestinationBuilder {
  public:
    explicit RenderDestinationBuilder(const Device& device, DescriptorRegistry& descriptorRegistry)
        : _device(device) {
        _textureFactory = std::make_unique<TextureFactory>(device, descriptorRegistry);
    };

    std::optional<RenderDestination> buildSwapchain(
        CommandManager& commandManager,
        GPUQueue& submitQueue,
        Swapchain& swapchain,
        RenderPassDescriptor& renderPassDescriptor
    ) const noexcept;

    std::optional<RenderDestination> buildMainOffscreen(
        CommandManager& commandManager,
        GPUQueue& submitQueue,
        VkExtent2D extent,
        RenderPassDescriptor& renderPassDescriptor
    ) const noexcept;

    std::optional<RenderDestination> buildShadowSunOffscreen(
        CommandManager& commandManager,
        GPUQueue& submitQueue,
        VkExtent2D extent,
        RenderPassDescriptor& renderPassDescriptor
    ) const noexcept;

  private:
    dev::Logger _logger = dev::Logger("RenderDestinationBuilder");

    std::reference_wrapper<const Device> _device;

    std::unique_ptr<vax::rhi::TextureFactory> _textureFactory = nullptr;
};
} // namespace vax::rhi