#pragma once

#include "device.h"
#include "gpuQueue.h"
#include "logger.h"
#include "vulkanCommon.h"

namespace vax::rhi {
class QueueManager final {
  public:
    QueueManager()
        : _ownerThreadId(std::this_thread::get_id()) {}

    QueueManager(const QueueManager& other) = delete;
    QueueManager(QueueManager&& other) = delete;
    QueueManager& operator=(const QueueManager& other) = delete;
    QueueManager& operator=(QueueManager&& other) = delete;

    void setup(const Device& device);

    GPUQueue& getMainGraphicsQueue() { return *_mainGraphicsQueue; }

    GPUQueue& getMainPresentQueue() { return *_mainPresentQueue; }

    GPUQueue& getMainComputeQueue() { return *_mainComputeQueue; }

    GPUQueue& getMainTransferQueue() { return *_mainTransferQueue; }

    GPUQueue& getSecondaryGraphicsQueue() { return *_secondaryGraphicsQueue; }

    GPUQueue& getSecondaryComputeQueue() { return *_secondaryComputeQueue; }

    GPUQueue& getSecondaryTransferQueue() { return *_secondaryTransferQueue; }

    // Not thread safe, borrow must happen on main thread! But queue becomes thread safe after borrow.
    GPUQueueHandle borrowMainGraphicsQueue() {
        assert(std::this_thread::get_id() == _ownerThreadId && "Borrow must happen on main thread!");
        return GPUQueueHandle::borrow(*_mainGraphicsQueue);
    }

    // Not thread safe, borrow must happen on main thread! But queue becomes thread safe after borrow.
    GPUQueueHandle borrowMainPresentQueue() {
        assert(std::this_thread::get_id() == _ownerThreadId && "Borrow must happen on main thread!");
        return GPUQueueHandle::borrow(*_mainPresentQueue);
    }

    // Not thread safe, borrow must happen on main thread! But queue becomes thread safe after borrow.
    GPUQueueHandle borrowMainComputeQueue() {
        assert(std::this_thread::get_id() == _ownerThreadId && "Borrow must happen on main thread!");
        return GPUQueueHandle::borrow(*_mainComputeQueue);
    }

    // Not thread safe, borrow must happen on main thread! But queue becomes thread safe after borrow.
    GPUQueueHandle borrowMainTransferQueue() {
        assert(std::this_thread::get_id() == _ownerThreadId && "Borrow must happen on main thread!");
        return GPUQueueHandle::borrow(*_mainTransferQueue);
    }

    GPUQueueHandle borrowSecondaryGraphicsQueue() { return GPUQueueHandle::borrow(*_secondaryGraphicsQueue); }

    GPUQueueHandle borrowSecondaryComputeQueue() { return GPUQueueHandle::borrow(*_secondaryComputeQueue); }

    GPUQueueHandle borrowSecondaryTransferQueue() { return GPUQueueHandle::borrow(*_secondaryTransferQueue); }

  private:
    dev::Logger _logger = dev::Logger("QueueManager");

    std::thread::id _ownerThreadId;

    struct QueueSlot {
        uint32_t family = 0;
        uint32_t index = 0;
        std::unique_ptr<GPUQueue> queue;
    };

    std::vector<QueueSlot> _queues;
    std::vector<uint32_t> _nextQueueIndex;

    GPUQueue* _mainGraphicsQueue = nullptr;
    GPUQueue* _mainPresentQueue = nullptr;
    GPUQueue* _mainComputeQueue = nullptr;
    GPUQueue* _mainTransferQueue = nullptr;

    GPUQueue* _secondaryGraphicsQueue = nullptr;
    GPUQueue* _secondaryComputeQueue = nullptr;
    GPUQueue* _secondaryTransferQueue = nullptr;

    QueueSlot& _getOrCreateQueue(const Device& device, uint32_t family);
};
} // namespace vax::rhi