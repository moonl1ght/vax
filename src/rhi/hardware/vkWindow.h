#pragma once

#include "logger.h"
#include "swapchain.h"
#include "vulkanCommon.h"
#include <SDL3/SDL.h>

namespace vax::rhi {

class VulkanWindow {
  public:
    VulkanWindow() noexcept = default;
    ~VulkanWindow() = default;

    VulkanWindow(const VulkanWindow& other) = delete;
    VulkanWindow(VulkanWindow&& other) noexcept = delete;
    VulkanWindow& operator=(const VulkanWindow& other) = delete;
    VulkanWindow& operator=(VulkanWindow&& other) noexcept = delete;

    bool setup(VkInstance instance, SDL_Window* window) noexcept;

    bool createSwapchain(
        const vax::rhi::Device& device, uint32_t width, uint32_t height, VkPresentModeKHR presentMode
    ) noexcept;

    void cleanup() noexcept;

    const vax::rhi::Swapchain* swapchain() const { return _swapchain.get(); }

    vax::rhi::Swapchain* swapchain() { return _swapchain.get(); }

    VkSurfaceKHR surface() const { return _surface; }

  private:
    dev::Logger _logger = dev::Logger("VulkanWindow");

    VkInstance _instance = VK_NULL_HANDLE;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    std::unique_ptr<rhi::Swapchain> _swapchain;
};

} // namespace vax::rhi