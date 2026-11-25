#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

namespace vax::rhi {
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr int MAX_MATERIALS = 500;
constexpr int MAX_DRAWABLE_INSTANCES = 2000;
constexpr uint32_t QUEUES_PER_ROLE = 2;
} // namespace vax::rhi

#define VK_CHECK(x)                                                                                                    \
    ({                                                                                                                 \
        VkResult err = (x);                                                                                            \
        if (err != VK_SUCCESS) {                                                                                       \
            vax::dev::Logger::getInstance().error("Detected Vulkan error: {}", err);                                   \
        }                                                                                                              \
        (err == VK_SUCCESS);                                                                                           \
    })

namespace vax::rhi {

inline PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT = nullptr;
inline PFN_vkCmdBeginDebugUtilsLabelEXT pfnCmdBeginDebugUtilsLabelEXT = nullptr;
inline PFN_vkCmdEndDebugUtilsLabelEXT pfnCmdEndDebugUtilsLabelEXT = nullptr;

inline void setPfnSetDebugUtilsObjectNameEXT(VkInstance instance) {
    if (pfnSetDebugUtilsObjectNameEXT == nullptr) {
        pfnSetDebugUtilsObjectNameEXT =
            (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
    }
    if (pfnCmdBeginDebugUtilsLabelEXT == nullptr) {
        pfnCmdBeginDebugUtilsLabelEXT =
            (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
    }
    if (pfnCmdEndDebugUtilsLabelEXT == nullptr) {
        pfnCmdEndDebugUtilsLabelEXT =
            (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
    }
}

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamily {
    uint32_t index = 0;
    VkQueueFlags flags = 0;
    uint32_t queueCount = 0;
    VkExtent3D minImageTransferGranularity{};
    bool supportsPresent = false;
    uint32_t createdQueueCount = 0;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> transferFamily;

    std::vector<QueueFamily> families;

    bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }

    uint32_t queueCount(uint32_t familyIndex) const { return families[familyIndex].queueCount; }

    std::vector<uint32_t> uniqueFamilies() const {
        std::vector<uint32_t> result;
        for (const auto& family : {graphicsFamily, presentFamily, computeFamily, transferFamily}) {
            if (family.has_value() && std::find(result.begin(), result.end(), *family) == result.end()) {
                result.push_back(*family);
            }
        }
        return result;
    }
};

uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

VkFormat findSupportedFormat(
    VkPhysicalDevice physicalDevice,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features
);
} // namespace vax::rhi