#pragma once

#include "vulkanCommon.h"
#include "logger.h"
#include <cstdint>
#include "vkUtils.h"

namespace vax::vk {
class Device final {
  public:
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice vkDevice = VK_NULL_HANDLE;
    VmaAllocator allocator = VK_NULL_HANDLE;

    Device() {}

    Device(const Device& other) = delete;
    Device(Device&& other) = delete;
    Device& operator=(const Device& other) = delete;
    Device& operator=(Device&& other) = delete;

    bool load(VkInstance instance, VkSurfaceKHR surface, bool enableValidationLayers, uint32_t vulkanApiVersion);
    void destroy();

    QueueFamilyIndices getQueueFamilyIndices() const { return _indices; }

    VkPhysicalDeviceProperties getPhysicalDeviceProperties() const;

    template <typename T> size_t minUniformBufferOffsetAlignment() const {
        VkDeviceSize minAlignment = getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
        return (sizeof(T) + minAlignment - 1) & ~(minAlignment - 1);
    }

  private:
    vax::Logger _logger = vax::Logger("Device");

    QueueFamilyIndices _indices;

    int createLogicalDevice(
        const VkPhysicalDevice& physicalDevice,
        const VkSurfaceKHR& surface,
        VkDevice& device,
        bool enableValidationLayers
    );

    bool isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

    int pickPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface, VkPhysicalDevice& physicalDevice);
};
} // namespace vax::vk