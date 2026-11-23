#include "vkEngine.h"
#include "vkDebug.h"
#include "vkInstanceBuilder.h"

using namespace vax::rhi;

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void validationLayersDestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

bool vax::rhi::VulkanEngine::createInstance() noexcept {
    std::optional<VkInstance> instanceOptional =
        VkInstanceBuilder(deletionQueue, enableValidationLayers, validationLayers, vulkanApiVersion).build();
    if (instanceOptional.has_value()) {
        instance = *instanceOptional;
    } else {
        _logger.error("Failed to create Vulkan instance!");
        return false;
    }
    deletionQueue.push_function([&]() {
        _logger.debug("Destroying instance...");
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    });
    vax::rhi::setPfnSetDebugUtilsObjectNameEXT(instance);

    if (!setupDebugMessenger()) {
        _logger.error("Failed to setup debug messenger!");
        return false;
    }
    return true;
}

bool vax::rhi::VulkanEngine::setup(VkSurfaceKHR mainWindowSurface) noexcept {
    device = std::make_unique<Device>();
    auto deviceLoadResult = device->load(instance, mainWindowSurface, enableValidationLayers, vulkanApiVersion);
    if (!deviceLoadResult) {
        _logger.error("Failed to load device!");
        return false;
    }

    deletionQueue.push_function([&]() {
        _logger.debug("Destroying device...");
        device = nullptr;
    });

    queueManager = std::make_unique<QueueManager>();
    queueManager->setup(*device);

    deletionQueue.push_function([&]() { queueManager = nullptr; });

    commandManager = std::make_unique<CommandManager>(*device);
    if (!commandManager->setup()) {
        _logger.error("Failed to setup command manager!");
        return false;
    }
    deletionQueue.push_function([&]() { commandManager->cleanup(); });

    syncObjectsManager = std::make_unique<SyncObjectsManager>(*device);
    if (!syncObjectsManager->setup()) {
        _logger.error("Failed to setup sync objects manager!");
        return false;
    }
    deletionQueue.push_function([&]() { syncObjectsManager->cleanup(); });

    pipelineManager = std::make_unique<PipelineManager>(*device);
    if (!pipelineManager->setup()) {
        _logger.error("Failed to setup pipeline manager!");
        return false;
    }
    deletionQueue.push_function([&]() {
        _logger.debug("Destroying pipeline manager...");
        pipelineManager = nullptr; // TODO: remove from destructor
    });

    _logger.success("Engine setup complete!");
    return true;
}

void vax::rhi::VulkanEngine::cleanup() {
    deletionQueue.flush();
    _logger.success("Vulkan engine cleanup complete!");
}

void vax::rhi::VulkanEngine::resize() {
    _logger.debug("Resizing engine...");
    // int width = 0, height = 0;
    // SDL_GetWindowSizeInPixels(_windowController.get().getWindow(0)->window, &width, &height);
    // while (width == 0 || height == 0) {
    //     SDL_GetWindowSizeInPixels(_windowController.get().getWindow(0)->window, &width, &height);
    //     SDL_Delay(100);
    // }
    // vkDeviceWaitIdle(device->vkDevice());

    // _windowController.get().getWindow(0)->getSwapchain()->recreate();
}

bool vax::rhi::VulkanEngine::setupDebugMessenger() {
    if (!enableValidationLayers)
        return false;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    vax::rhi::populateDebugMessengerCreateInfo(createInfo);

    if (!VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger))) {
        _logger.error("Failed to set up debug messenger!");
        return false;
    }

    deletionQueue.push_function([&]() {
        _logger.debug("Destroying debug messenger...");
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    });

    return true;
}