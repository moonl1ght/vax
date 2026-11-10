#include "device.h"
#include <set>

using namespace vax::vk;

std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME
};

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Device::destroy() {
    _logger.info("Destroying device...");
    vkDestroyDevice(vkDevice, nullptr);
    vkDevice = VK_NULL_HANDLE;
    _indices = QueueFamilyIndices();
}

int Device::createLogicalDevice(
    const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, VkDevice& device, bool enableValidationLayers
) {

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies{_indices.graphicsFamily.value(), _indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .pNext = nullptr,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 deviceFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &descriptorIndexingFeatures,
        .features = {.multiDrawIndirect = VK_TRUE, .samplerAnisotropy = VK_TRUE},
    };

    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
        .pNext = &deviceFeatures,
        .bufferDeviceAddress = VK_TRUE,
    };

    VkPhysicalDeviceSynchronization2Features synchronization2Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
        .pNext = &bufferDeviceAddressFeatures,
        .synchronization2 = VK_TRUE,
    };

    VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR,
        .pNext = &synchronization2Features,
        .fragmentShaderBarycentric = VK_TRUE,
    };

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = nullptr;
    createInfo.pNext = &fragmentShaderBarycentricFeatures;

    createInfo.enabledExtensionCount = 0;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        _logger.error("failed to create logical device!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

bool Device::isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

int Device::pickPhysicalDevice(
    const VkInstance& instance, const VkSurfaceKHR& surface, VkPhysicalDevice& physicalDevice
) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        _logger.error("failed to find GPUs with Vulkan support!");
        return EXIT_FAILURE;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device, surface)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        _logger.error("failed to find a suitable GPU!");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

bool Device::load(VkInstance instance, VkSurfaceKHR surface, bool enableValidationLayers, uint32_t vulkanApiVersion) {
    _logger.info("Loading device...");
    if (pickPhysicalDevice(instance, surface, vkPhysicalDevice) == EXIT_SUCCESS) {
        _indices = findQueueFamilies(vkPhysicalDevice, surface);
        if (createLogicalDevice(vkPhysicalDevice, surface, vkDevice, enableValidationLayers) == EXIT_SUCCESS) {
            _logger.info("Device loaded successfully!");
            VmaAllocatorCreateInfo allocatorInfo{
                .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
                .physicalDevice = vkPhysicalDevice,
                .device = vkDevice,
                .instance = instance,
                .vulkanApiVersion = vulkanApiVersion,
            };
            vmaCreateAllocator(&allocatorInfo, &allocator);
            return true;
        }
    }
    _logger.error("Failed to load device!");
    return false;
}

VkPhysicalDeviceProperties Device::getPhysicalDeviceProperties() const {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(vkPhysicalDevice, &properties);
    return properties;
}
