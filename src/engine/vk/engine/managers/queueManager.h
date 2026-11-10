#pragma once

#include "device.h"
#include "vulkanCommon.h"
#include "logger.h"

namespace vax::vk {
class QueueManager final {
  public:
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    QueueManager() {}

    QueueManager(const QueueManager& other) = delete;
    QueueManager(QueueManager&& other) = delete;
    QueueManager& operator=(const QueueManager& other) = delete;
    QueueManager& operator=(QueueManager&& other) = delete;

    void setup(const Device& device);

  private:
    vax::Logger _logger = vax::Logger("QueueManager");
};
} // namespace vax::vk