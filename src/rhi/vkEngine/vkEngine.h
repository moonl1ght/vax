#pragma once

#include "commandManager.h"
#include "deletionQueue.h"
#include "device.h"
#include "logger.h"
#include "pipelineManager.h"
#include "queueManager.h"
#include "syncObjectsManager.h"
#include "vulkanCommon.h"
#include "windowController.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace vax::rhi {
class Device;
class QueueManager;
} // namespace vax::rhi

namespace vax::rhi {
class Engine final {
  public:
    const uint32_t vulkanApiVersion = VK_API_VERSION_1_4;

    const bool enableValidationLayers = true;

    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    bool framebufferResized = false;

    vax::core::DeletionQueue deletionQueue;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkInstance instance = VK_NULL_HANDLE;

    std::unique_ptr<vax::rhi::Device> device;

    std::unique_ptr<vax::rhi::QueueManager> queueManager;
    std::unique_ptr<vax::rhi::PipelineManager> pipelineManager;
    std::unique_ptr<vax::rhi::CommandManager> commandManager;
    std::unique_ptr<vax::rhi::SyncObjectsManager> syncObjectsManager;

    explicit Engine(app::WindowController& windowController)
        : _windowController(windowController) {};

    ~Engine() {};

    Engine(const Engine& other) = delete;
    Engine& operator=(const Engine& other) = delete;
    Engine(Engine&& other) noexcept = delete;
    Engine& operator=(Engine&& other) noexcept = delete;

    bool setup();
    void cleanup();
    void resize();

    app::WindowController& getWindowController() { return _windowController.get(); }

    const app::WindowController& getWindowController() const { return _windowController.get(); }

  private:
    dev::Logger _logger = dev::Logger("Engine");

    std::reference_wrapper<app::WindowController> _windowController;

    bool setupDebugMessenger();
};
} // namespace vax::rhi