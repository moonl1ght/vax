#pragma once

#include "device.h"
#include "vulkanCommon.h"
#include "logger.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace vax::rhi {
class SyncObjectsManager {
  public:
    SyncObjectsManager(const vax::rhi::Device& device)
        : _device(device) {};

    bool setup();

    bool cleanup();

    const std::vector<VkSemaphore>& getImageAvailableSemaphores(const std::string& name) const {
        return _imageAvailableSemaphores.at(name);
    }

    const std::vector<VkSemaphore>& getRenderFinishedSemaphores(const std::string& name) const {
        return _renderFinishedSemaphores.at(name);
    }

    const std::vector<VkFence>& getInFlightFences() const { return _inFlightFences; }

  private:
    std::unordered_map<std::string, std::vector<VkSemaphore>> _imageAvailableSemaphores;
    std::unordered_map<std::string, std::vector<VkSemaphore>> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;

    vax::Logger _logger = vax::Logger("SyncObjectsManager");
    std::reference_wrapper<const vax::rhi::Device> _device;
};
} // namespace vax::rhi