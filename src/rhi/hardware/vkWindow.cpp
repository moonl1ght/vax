#include "vkWindow.h"
#include <SDL3/SDL_vulkan.h>

using namespace vax::rhi;

bool VulkanWindow::setup(VkInstance instance, SDL_Window* window) noexcept {
    _instance = instance;
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &_surface)) {
        _logger.error("Failed to create surface");
        return false;
    }
    return true;
}

bool VulkanWindow::createSwapchain(
    const Device& device, uint32_t width, uint32_t height, VkPresentModeKHR presentMode
) noexcept {
    auto size = math::SizeUI{width, height};
    _swapchain = std::make_unique<Swapchain>(device, _surface, size);
    if (!_swapchain->setup(presentMode)) {
        _logger.error("Failed to create swapchain");
        return false;
    }
    return true;
}

void VulkanWindow::cleanup() noexcept {
    _logger.info("Cleaning up vulkan window...");
    if (_swapchain != nullptr) {
        _swapchain->cleanup();
        _swapchain = nullptr;
    }
    if (_surface != VK_NULL_HANDLE && _instance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        _surface = VK_NULL_HANDLE;
        _instance = VK_NULL_HANDLE;
    }
}