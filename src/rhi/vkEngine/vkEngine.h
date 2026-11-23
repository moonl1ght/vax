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
class VulkanEngine final {
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

    explicit VulkanEngine() noexcept = default;

    ~VulkanEngine() {};

    VulkanEngine(const VulkanEngine& other) = delete;
    VulkanEngine& operator=(const VulkanEngine& other) = delete;
    VulkanEngine(VulkanEngine&& other) noexcept = delete;
    VulkanEngine& operator=(VulkanEngine&& other) noexcept = delete;

    bool createInstance() noexcept;

    bool setup(VkSurfaceKHR mainWindowSurface) noexcept;

    void cleanup();

    void resize();

  private:
    dev::Logger _logger = dev::Logger("VulkanEngine");

    bool setupDebugMessenger();
};
} // namespace vax::rhi