#include "device.h"
#include <set>

using namespace vax::rhi;

std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME
};

uint32_t deviceTypeScore(VkPhysicalDeviceType type) {
    switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return 1000;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return 100;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return 50;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return 1;
    default:
        return 10;
    }
}

uint32_t deviceLocalMemoryGb(VkPhysicalDevice device) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(device, &memProps);
    VkDeviceSize total = 0;
    for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i) {
        if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            total += memProps.memoryHeaps[i].size;
        }
    }
    return static_cast<uint32_t>(total >> 30);
}

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

static QueueFamilyIndices findDeviceQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    if (surface != VK_NULL_HANDLE) {
        return findQueueFamilies(device, surface);
    }

    QueueFamilyIndices indices;
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

    for (uint32_t i = 0; i < count; ++i) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            break;
        }
    }
    return indices;
}

int Device::_createLogicalDevice(
    const VkPhysicalDevice& physicalDevice, VkDevice& device, bool enableValidationLayers
) noexcept {

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies{_indices.graphicsFamily.value()};
    if (_indices.presentFamily.has_value()) {
        uniqueQueueFamilies.insert(_indices.presentFamily.value());
    }

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

    VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR barycentricFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR,
        .pNext = nullptr,
        .fragmentShaderBarycentric = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features vulkan13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &barycentricFeatures,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceVulkan12Features vulkan12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &vulkan13Features,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
        .scalarBlockLayout = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE,
    };

    VkPhysicalDeviceVulkan11Features vulkan11Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &vulkan12Features,
        .shaderDrawParameters = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 deviceFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vulkan11Features,
        .features = {
        .multiDrawIndirect = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
        .shaderStorageImageReadWithoutFormat = VK_TRUE,
        .shaderStorageImageWriteWithoutFormat = VK_TRUE,
        .shaderInt64 = VK_TRUE,
        },
    };

    VkDeviceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &deviceFeatures,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = nullptr,
    };

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        _logger.error("failed to create logical device!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

bool Device::_isDeviceSuitable(const VkPhysicalDevice& device, VkSurfaceKHR surface) noexcept {
    QueueFamilyIndices indices = findDeviceQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    bool baseOk = indices.graphicsFamily.has_value() && extensionsSupported && supportedFeatures.samplerAnisotropy;
    if (!baseOk || surface == VK_NULL_HANDLE) {
        return baseOk;
    }

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
    bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    return indices.isComplete() && swapChainAdequate;
}

int Device::_pickPhysicalDevice(
    const VkInstance& instance, VkSurfaceKHR surface, VkPhysicalDevice& physicalDevice
) noexcept {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        _logger.error("failed to find GPUs with Vulkan support!");
        return EXIT_FAILURE;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    const char* overrideEnv = std::getenv("VAX_GPU");
    std::string_view overrideName = overrideEnv ? overrideEnv : "";

    VkPhysicalDevice best = VK_NULL_HANDLE;
    uint32_t bestScore = 0;

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        std::string_view name = props.deviceName;

        if (props.apiVersion < VK_API_VERSION_1_4) {
            _logger.debug("Skipping ", name, ": Vulkan 1.4 not supported");
            continue;
        }
        if (!_isDeviceSuitable(device, surface)) {
            _logger.debug("Skipping ", name, ": not suitable");
            continue;
        }

        uint32_t score = deviceTypeScore(props.deviceType) + deviceLocalMemoryGb(device);
        if (!overrideName.empty() && name.find(overrideName) != std::string_view::npos) {
            score += 100000;
        }

        if (score > bestScore) {
            bestScore = score;
            best = device;
        }
    }

    if (best == VK_NULL_HANDLE) {
        _logger.error("failed to find a suitable GPU!");
        return EXIT_FAILURE;
    }

    physicalDevice = best;
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    _logger.info(
        "Selected GPU: ",
        props.deviceName,
        " (Vulkan ",
        VK_API_VERSION_MAJOR(props.apiVersion),
        ".",
        VK_API_VERSION_MINOR(props.apiVersion),
        ".",
        VK_API_VERSION_PATCH(props.apiVersion),
        ")"
    );
    return EXIT_SUCCESS;
}

bool Device::load(
    VkInstance instance, VkSurfaceKHR surface, bool enableValidationLayers, uint32_t vulkanApiVersion
) noexcept {
    _logger.debug("Loading device", surface == VK_NULL_HANDLE ? " (headless)..." : "...");
    if (_pickPhysicalDevice(instance, surface, _vkPhysicalDevice) == EXIT_SUCCESS) {
        _indices = findDeviceQueueFamilies(_vkPhysicalDevice, surface);
        if (_createLogicalDevice(_vkPhysicalDevice, _vkDevice, enableValidationLayers) == EXIT_SUCCESS) {
            VmaAllocatorCreateInfo allocatorInfo{
                .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
                .physicalDevice = _vkPhysicalDevice,
                .device = _vkDevice,
                .instance = instance,
                .vulkanApiVersion = vulkanApiVersion,
            };
            vmaCreateAllocator(&allocatorInfo, &_allocator);
            _loadDeviceProps();
            return true;
        }
    }
    return false;
}

void Device::_loadDeviceProps() {
    _vulkan12Properties =
        VkPhysicalDeviceVulkan12Properties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES};
    VkPhysicalDeviceProperties2 properties2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &_vulkan12Properties,
    };
    vkGetPhysicalDeviceProperties2(_vkPhysicalDevice, &properties2);
    _properties = properties2.properties;
    _vulkan12Properties.pNext = nullptr;
}
