#include "vkUtils.h"

uint32_t vax::rhi::findMemoryType(
    const VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties
) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

vax::rhi::SwapChainSupportDetails
vax::rhi::querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
    vax::rhi::SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

vax::rhi::QueueFamilyIndices vax::rhi::findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
    vax::rhi::QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    indices.families.reserve(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        VkBool32 presentSupport = VK_FALSE;
        if (surface != VK_NULL_HANDLE) {
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        }
        indices.families.push_back({
            .index = i,
            .flags = queueFamilies[i].queueFlags,
            .queueCount = queueFamilies[i].queueCount,
            .minImageTransferGranularity = queueFamilies[i].minImageTransferGranularity,
            .supportsPresent = presentSupport == VK_TRUE,
        });
    }

    auto has = [](const QueueFamily& family, VkQueueFlags flags) { return (family.flags & flags) != 0; };
    auto findFamily = [&](auto&& predicate) -> std::optional<uint32_t> {
        for (const auto& family : indices.families) {
            if (family.queueCount > 0 && predicate(family)) {
                return family.index;
            }
        }
        return std::nullopt;
    };

    indices.graphicsFamily =
        findFamily([&](const QueueFamily& f) { return has(f, VK_QUEUE_GRAPHICS_BIT) && f.supportsPresent; });
    if (!indices.graphicsFamily) {
        indices.graphicsFamily = findFamily([&](const QueueFamily& f) { return has(f, VK_QUEUE_GRAPHICS_BIT); });
    }

    if (indices.graphicsFamily && indices.families[*indices.graphicsFamily].supportsPresent) {
        indices.presentFamily = indices.graphicsFamily;
    } else {
        indices.presentFamily = findFamily([](const QueueFamily& f) { return f.supportsPresent; });
    }

    indices.computeFamily = findFamily([&](const QueueFamily& f) {
        return has(f, VK_QUEUE_COMPUTE_BIT) && !has(f, VK_QUEUE_GRAPHICS_BIT);
    });
    if (!indices.computeFamily) {
        indices.computeFamily = findFamily([&](const QueueFamily& f) { return has(f, VK_QUEUE_COMPUTE_BIT); });
    }

    indices.transferFamily = findFamily([&](const QueueFamily& f) {
        return has(f, VK_QUEUE_TRANSFER_BIT) && !has(f, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    });
    if (!indices.transferFamily) {
        indices.transferFamily = findFamily([&](const QueueFamily& f) {
            return has(f, VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT) && !has(f, VK_QUEUE_GRAPHICS_BIT);
        });
    }
    if (!indices.transferFamily) {
        indices.transferFamily = indices.graphicsFamily;
    }

    return indices;
}

VkFormat vax::rhi::findSupportedFormat(
    VkPhysicalDevice physicalDevice,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features
) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}