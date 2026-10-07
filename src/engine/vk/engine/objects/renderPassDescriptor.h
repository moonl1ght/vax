#pragma once

#include "device.h"
#include "luna.h"
#include <vector>

namespace vax::vk {
class RenderPassDescriptor final {
  public:
    VkFormat imageFormat;
    VkFormat depthFormat;
    uint32_t colorAttachmentCount;
    std::vector<VkClearValue> colorClearValues;

    explicit RenderPassDescriptor(
        const vax::vk::Device& device,
        VkRenderPass renderPass,
        VkFormat imageFormat,
        VkFormat depthFormat,
        uint32_t colorAttachmentCount = 1,
        std::vector<VkClearValue> colorClearValues = {}
    )
        : _device(device)
        , _renderPass(renderPass)
        , imageFormat(imageFormat)
        , depthFormat(depthFormat)
        , colorAttachmentCount(colorAttachmentCount)
        , colorClearValues(std::move(colorClearValues)) {};

    RenderPassDescriptor(const RenderPassDescriptor& other) = delete;
    RenderPassDescriptor& operator=(const RenderPassDescriptor& other) = delete;

    RenderPassDescriptor(RenderPassDescriptor&& other) noexcept
        : _device(other._device)
        , _renderPass(other._renderPass)
        , imageFormat(other.imageFormat)
        , depthFormat(other.depthFormat)
        , colorAttachmentCount(other.colorAttachmentCount)
        , colorClearValues(std::move(other.colorClearValues)) {
        other._renderPass = VK_NULL_HANDLE;
    }

    RenderPassDescriptor& operator=(RenderPassDescriptor&& other) noexcept {
        if (this != &other) {
            vkDestroyRenderPass(_device.get().vkDevice, _renderPass, nullptr);
            _device = other._device;
            _renderPass = other._renderPass;
            imageFormat = other.imageFormat;
            depthFormat = other.depthFormat;
            colorAttachmentCount = other.colorAttachmentCount;
            colorClearValues = std::move(other.colorClearValues);
            other._renderPass = VK_NULL_HANDLE;
        }
        return *this;
    }

    ~RenderPassDescriptor() { vkDestroyRenderPass(_device.get().vkDevice, _renderPass, nullptr); };

    const VkRenderPass getVkRenderPass() const { return _renderPass; }

  private:
    std::reference_wrapper<const vax::vk::Device> _device;
    VkRenderPass _renderPass = VK_NULL_HANDLE;
};
} // namespace vax::vk