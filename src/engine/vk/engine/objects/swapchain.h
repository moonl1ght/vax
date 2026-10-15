#pragma once

#include "device.h"
#include "luna.h"
#include "size.h"
#include "vkUtils.h"

namespace vax::vk {
class Swapchain final {
  public:
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkFormat swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent = {0, 0};

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    explicit Swapchain(const vax::vk::Device& device, VkSurfaceKHR surface, vax::math::SizeUI size)
        : _surface(surface)
        , _size(size)
        , _device(device) {};

    Swapchain(const Swapchain& other) = delete;
    Swapchain(Swapchain&& other) = delete;
    Swapchain& operator=(const Swapchain& other) = delete;
    Swapchain& operator=(Swapchain&& other) = delete;

    bool setup(VkPresentModeKHR presentMode);
    void cleanup();
    bool recreate() { return _recreate(_requestedPresentMode); }

  private:
    vax::Logger _logger = vax::Logger("Swapchain");
    vax::math::SizeUI _size;
    VkPresentModeKHR _requestedPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    std::reference_wrapper<const vax::vk::Device> _device;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;

    bool _createSwapchain(VkPresentModeKHR requestedPresentMode);
    bool _createImageViews();

    bool _recreate(VkPresentModeKHR presentMode);

    VkSurfaceFormatKHR _chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkExtent2D _chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkPresentModeKHR _choosePresentMode(
        VkPresentModeKHR requestedPresentMode, const std::vector<VkPresentModeKHR>& availablePresentModes
    );
};
} // namespace vax::vk