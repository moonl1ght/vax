#pragma once

#include "commandManager.h"
#include "deletionQueue.h"
#include "descriptorSetManager.h"
#include "device.h"
#include "luna.h"
#include "pipelineManager.h"
#include "queueManager.h"
#include "syncObjectsManager.h"
#include "windowController.h"
#include "shaderLibrary.h"

namespace vax::vk {
class Device;
class QueueManager;
} // namespace vax::vk

namespace vax::vk {
class Engine final {
  public:
    const uint32_t vulkanApiVersion = VK_API_VERSION_1_4;

    const bool enableValidationLayers = true;

    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    bool framebufferResized = false;

    vax::core::DeletionQueue deletionQueue;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkInstance instance = VK_NULL_HANDLE;

    std::unique_ptr<vax::vk::Device> device;
    std::unique_ptr<vax::vk::ShaderLibrary> shaderLibrary;

    std::unique_ptr<vax::vk::QueueManager> queueManager;
    std::unique_ptr<vax::vk::DescriptorSetManager> descriptorSetManager;
    std::unique_ptr<vax::vk::PipelineManager> pipelineManager;
    std::unique_ptr<vax::vk::CommandManager> commandManager;
    std::unique_ptr<vax::vk::SyncObjectsManager> syncObjectsManager;

    explicit Engine(vax::WindowController& windowController)
        : _windowController(windowController) {};

    ~Engine() {};

    Engine(const Engine& other) = delete;
    Engine& operator=(const Engine& other) = delete;
    Engine(Engine&& other) noexcept = delete;
    Engine& operator=(Engine&& other) noexcept = delete;

    bool setup();
    void cleanup();
    void resize();

    vax::WindowController& getWindowController() { return _windowController.get(); }

    const vax::WindowController& getWindowController() const { return _windowController.get(); }

  private:
    vax::Logger _logger = vax::Logger("Engine");

    std::reference_wrapper<vax::WindowController> _windowController;

    bool setupDebugMessenger();
};
} // namespace vax::vk