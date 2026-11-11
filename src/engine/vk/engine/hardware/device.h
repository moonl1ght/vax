#pragma once

#include "logger.h"
#include "vkUtils.h"
#include <cstdint>

namespace vax::vk {
class Device final {
  public:
    explicit Device() noexcept = default;

    ~Device() {
        vmaDestroyAllocator(_allocator);
        vkDestroyDevice(_vkDevice, nullptr);
        _vkPhysicalDevice = VK_NULL_HANDLE;
        _vkDevice = VK_NULL_HANDLE;
        _allocator = VK_NULL_HANDLE;
        _indices = QueueFamilyIndices();
    }

    Device(const Device& other) = delete;
    Device(Device&& other) = delete;
    Device& operator=(const Device& other) = delete;
    Device& operator=(Device&& other) = delete;

    bool
    load(VkInstance instance, VkSurfaceKHR surface, bool enableValidationLayers, uint32_t vulkanApiVersion) noexcept;

    QueueFamilyIndices queueFamilyIndices() const noexcept { return _indices; }

    VkPhysicalDeviceProperties physicalDeviceProperties() const noexcept { return _properties; }

    template <typename T> size_t minUniformBufferOffsetAlignment() const noexcept {
        VkDeviceSize minAlignment = physicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
        return (sizeof(T) + minAlignment - 1) & ~(minAlignment - 1);
    }

    VmaAllocator allocator() const noexcept { return _allocator; }

    VkDevice vkDevice() const noexcept { return _vkDevice; }

    VkPhysicalDevice vkPhysicalDevice() const noexcept { return _vkPhysicalDevice; }

  private:
    vax::Logger _logger = vax::Logger("Device");

    QueueFamilyIndices _indices;
    VkPhysicalDeviceProperties _properties;
    VmaAllocator _allocator = VK_NULL_HANDLE;
    VkDevice _vkDevice = VK_NULL_HANDLE;
    VkPhysicalDevice _vkPhysicalDevice = VK_NULL_HANDLE;

    int _createLogicalDevice(
        const VkPhysicalDevice& physicalDevice,
        const VkSurfaceKHR& surface,
        VkDevice& device,
        bool enableValidationLayers
    ) noexcept;

    bool _isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) noexcept;

    int _pickPhysicalDevice(
        const VkInstance& instance, const VkSurfaceKHR& surface, VkPhysicalDevice& physicalDevice
    ) noexcept;

    void _loadDeviceProps();
};
} // namespace vax::vk