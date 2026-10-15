#include "vkEngine.h"
#include "vkDebug.h"
#include "vkInstanceBuilder.h"

using namespace vax::vk;

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

bool vax::vk::Engine::setup() {
    std::optional<VkInstance> instanceOptional =
        VkInstanceBuilder(deletionQueue, enableValidationLayers, validationLayers, vulkanApiVersion).build();
    if (instanceOptional.has_value()) {
        instance = *instanceOptional;
    } else {
        return false;
    }
    deletionQueue.push_function([&]() {
        _logger.debug("Destroying instance...");
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    });
    vax::vk::setPfnSetDebugUtilsObjectNameEXT(instance);

    if (!setupDebugMessenger())
        return false;

    _windowController.get().getWindow(0)->createSurface(instance);
    deletionQueue.push_function([&]() { _windowController.get().getWindow(0)->destroySurface(); });

    device = std::make_unique<Device>();
    if (!device->load(
            instance, _windowController.get().getWindow(0)->surface, enableValidationLayers, vulkanApiVersion
        )) {
        return false;
    }

    deletionQueue.push_function([&]() { device->destroy(); });

    queueManager = std::make_unique<QueueManager>();
    queueManager->setup(*device);

    deletionQueue.push_function([&]() { queueManager = nullptr; });

    commandManager = std::make_unique<CommandManager>(*device);
    if (!commandManager->setup())
        return false;
    deletionQueue.push_function([&]() { commandManager->cleanup(); });

    syncObjectsManager = std::make_unique<SyncObjectsManager>(*device);
    if (!syncObjectsManager->setup())
        return false;
    deletionQueue.push_function([&]() { syncObjectsManager->cleanup(); });

    _windowController.get().getWindow(0)->createSwapchain(*device, VK_PRESENT_MODE_FIFO_KHR);
    deletionQueue.push_function([&]() { _windowController.get().getWindow(0)->cleanupSwapchain(); });

    descriptorSetManager = std::make_unique<DescriptorSetManager>(*device, MAX_FRAMES_IN_FLIGHT);
    auto descriptorSetInfos = allDescriptorSets();
    if (!descriptorSetManager->setup(descriptorSetInfos)) {
        return false;
    }
    deletionQueue.push_function([&]() {
        _logger.debug("Destroying descriptor set manager...");
        descriptorSetManager->cleanup();
        descriptorSetManager = nullptr;
    });

    pipelineManager = std::make_unique<PipelineManager>(*device, *descriptorSetManager);
    deletionQueue.push_function([&]() {
        _logger.debug("Destroying pipeline manager...");
        pipelineManager = nullptr; // TODO: remove from destructor
    });

    _logger.info("Engine setup complete!");
    return true;
}

void vax::vk::Engine::cleanup() {
    deletionQueue.flush();
    _logger.info("Engine cleanup complete!");
}

void vax::vk::Engine::resize() {
    _logger.info("Resizing engine...");
    int width = 0, height = 0;
    SDL_GetWindowSizeInPixels(_windowController.get().getWindow(0)->window, &width, &height);
    while (width == 0 || height == 0) {
        SDL_GetWindowSizeInPixels(_windowController.get().getWindow(0)->window, &width, &height);
        SDL_Delay(100);
    }
    vkDeviceWaitIdle(device->vkDevice);

    _windowController.get().getWindow(0)->getSwapchain()->recreate();
}

bool vax::vk::Engine::setupDebugMessenger() {
    LOG_INFO("Setting up debug messenger...");
    if (!enableValidationLayers)
        return false;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    vax::vk::populateDebugMessengerCreateInfo(createInfo);

    if (!VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger))) {
        LOG_ERROR("Failed to set up debug messenger!");
        return false;
    }

    deletionQueue.push_function([&]() {
        LOG_INFO("Destroying debug messenger...");
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    });

    return true;
}