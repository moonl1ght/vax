#pragma once

#include "device.h"
#include "renderPassDescriptor.h"

namespace vax::rhi {
class RenderPassDescriptorBuilder {
  public:
    explicit RenderPassDescriptorBuilder(const vax::rhi::Device& device)
        : _device(device) {};

    std::unique_ptr<vax::rhi::RenderPassDescriptor> buildSwapchain(VkFormat imageFormat) const noexcept;

    std::unique_ptr<vax::rhi::RenderPassDescriptor>
    buildOffscreen(VkFormat imageFormat, bool allowComputeUsage) const noexcept;

    std::unique_ptr<vax::rhi::RenderPassDescriptor>
    buildShadowSun(VkFormat imageFormat, bool allowComputeUsage) const noexcept;

  private:
    dev::Logger _logger = dev::Logger("RenderPassDescriptorBuilder");
    std::reference_wrapper<const vax::rhi::Device> _device;
};
} // namespace vax::rhi