#include "swapchain.h"
#include "imageUtils.h"

using namespace vax::vk;

bool Swapchain::setup(VkPresentModeKHR presentMode) {
    if (!_createSwapchain(presentMode))
        return false;
    if (!_createImageViews())
        return false;
    return true;
}

void Swapchain::cleanup() {
    _logger.debug("Cleaning up swapchain...");
    for (auto imageView : swapchainImageViews) {
        vkDestroyImageView(_device.get().vkDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(_device.get().vkDevice, swapchain, nullptr);
}

bool Swapchain::_recreate(VkPresentModeKHR presentMode) {
    cleanup();

    if (!setup(presentMode))
        return false;

    return true;
}

VkSurfaceFormatKHR Swapchain::_chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkExtent2D Swapchain::_chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {static_cast<uint32_t>(_size.width), static_cast<uint32_t>(_size.height)};

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkPresentModeKHR Swapchain::_choosePresentMode(
    VkPresentModeKHR requestedPresentMode, const std::vector<VkPresentModeKHR>& availablePresentModes
) {
    auto isAvailable = [&](VkPresentModeKHR mode) {
        return std::ranges::find(availablePresentModes, mode) != availablePresentModes.end();
    };
    if (isAvailable(requestedPresentMode)) {
        return requestedPresentMode;
    }
    if (requestedPresentMode == VK_PRESENT_MODE_MAILBOX_KHR || requestedPresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        for (VkPresentModeKHR mode : {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR}) {
            if (isAvailable(mode)) {
                _logger.warning("Requested present mode ", requestedPresentMode, " is not supported, using ", mode);
                return mode;
            }
        }
    }
    _logger.warning("Requested present mode ", requestedPresentMode, " is not supported, falling back to FIFO");
    return VK_PRESENT_MODE_FIFO_KHR;
}

bool Swapchain::_createSwapchain(VkPresentModeKHR requestedPresentMode) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_device.get().vkPhysicalDevice, _surface);

    _requestedPresentMode = requestedPresentMode;
    presentMode = _choosePresentMode(requestedPresentMode, swapChainSupport.presentModes);

    VkSurfaceFormatKHR surfaceFormat = _chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkExtent2D extent = _chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = _surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    QueueFamilyIndices indices = findQueueFamilies(_device.get().vkPhysicalDevice, _surface);
    if (!indices.isComplete()) {
        _logger.error("Queue family indices are not complete!");
        return false;
    }

    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    if (!VK_CHECK(vkCreateSwapchainKHR(_device.get().vkDevice, &createInfo, nullptr, &swapchain))) {
        _logger.error("Failed to create swap chain!");
        return false;
    }

    vkGetSwapchainImagesKHR(_device.get().vkDevice, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    if (!VK_CHECK(vkGetSwapchainImagesKHR(_device.get().vkDevice, swapchain, &imageCount, swapchainImages.data()))) {
        _logger.error("Failed to get swap chain images!");
        return false;
    }

    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent = extent;

    return true;
}

bool Swapchain::_createImageViews() {
    swapchainImageViews.resize(swapchainImages.size());

    for (uint32_t i = 0; i < swapchainImages.size(); i++) {
        auto swapchainImageView = createImageView(
            _device.get().vkDevice, swapchainImages[i], swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT
        );
        if (swapchainImageView) {
            swapchainImageViews[i] = *swapchainImageView;
        } else {
            _logger.error("Failed to create swap chain image view!");
            return false;
        }
    }

    return true;
}