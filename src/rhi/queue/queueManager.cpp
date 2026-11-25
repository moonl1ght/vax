#include "queueManager.h"
#include "vkUtils.h"

using namespace vax::rhi;

void QueueManager::setup(const Device& device) {
    const QueueFamilyIndices indices = device.queueFamilyIndices();
    _queues.clear();
    _nextQueueIndex.assign(indices.families.size(), 0);

    _mainGraphicsQueue = _getOrCreateQueue(device, indices.graphicsFamily.value()).queue.get();
    _mainComputeQueue = _getOrCreateQueue(device, indices.computeFamily.value()).queue.get();
    _mainTransferQueue = _getOrCreateQueue(device, indices.transferFamily.value()).queue.get();

    _secondaryGraphicsQueue = _getOrCreateQueue(device, indices.graphicsFamily.value()).queue.get();
    _secondaryComputeQueue = _getOrCreateQueue(device, indices.computeFamily.value()).queue.get();
    _secondaryTransferQueue = _getOrCreateQueue(device, indices.transferFamily.value()).queue.get();

    if (indices.presentFamily.has_value()) {
        if (indices.presentFamily == indices.graphicsFamily) {
            _mainPresentQueue = _mainGraphicsQueue;
        } else {
            QueueSlot& present = _getOrCreateQueue(device, indices.presentFamily.value());
            _mainPresentQueue = present.queue.get();
        }
    }
}

QueueManager::QueueSlot& QueueManager::_getOrCreateQueue(const Device& device, uint32_t family) {
    const uint32_t createdQueueCount = device.queueFamilyIndices().families[family].createdQueueCount;
    assert(createdQueueCount > 0 && "Queue family was not created on the device!");
    const uint32_t index = _nextQueueIndex[family]++ % createdQueueCount;

    for (auto& slot : _queues) {
        if (slot.family == family && slot.index == index) {
            _logger.debug("Queue family ", family, " index ", index, " is shared between roles");
            return slot;
        }
    }

    VkQueue vkQueue = VK_NULL_HANDLE;
    vkGetDeviceQueue(device.vkDevice(), family, index, &vkQueue);
    _queues.push_back({
        .family = family,
        .index = index,
        .queue = std::make_unique<GPUQueue>(vkQueue),
    });
    return _queues.back();
}
