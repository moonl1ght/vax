#pragma once

#include "commandManager.h"
#include "device.h"
#include "logger.h"
#include "renderDestination.h"
#include "renderPassDescriptor.h"
#include "swapchain.h"
#include <optional>

namespace vax::rhi {
class RenderDestinationBuilder {
  public:
    explicit RenderDestinationBuilder(const Device& device, TextureManager& textureManager)
        : _device(device)
        , _textureManager(textureManager) {};

    std::optional<RenderDestination> buildSwapchain(
        CommandManager& commandManager,
        VkQueue submitQueue,
        Swapchain& swapchain,
        RenderPassDescriptor& renderPassDescriptor
    ) const noexcept;

    std::optional<RenderDestination> buildMainOffscreen(
        CommandManager& commandManager,
        VkQueue submitQueue,
        VkExtent2D extent,
        RenderPassDescriptor& renderPassDescriptor
    ) const noexcept;

    std::optional<RenderDestination> buildShadowSunOffscreen(
        CommandManager& commandManager,
        VkQueue submitQueue,
        VkExtent2D extent,
        RenderPassDescriptor& renderPassDescriptor
    ) const noexcept;

  private:
    vax::Logger _logger = vax::Logger("RenderDestinationBuilder");

    std::reference_wrapper<const Device> _device;
    std::reference_wrapper<vax::rhi::TextureManager> _textureManager;
};
} // namespace vax::rhi