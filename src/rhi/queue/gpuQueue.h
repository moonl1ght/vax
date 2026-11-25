#pragma once

#include "vulkanCommon.h"
#include <mutex>

namespace vax::rhi {

class GPUQueue final {
  public:
    friend class GPUQueueHandle;

    explicit GPUQueue(VkQueue queue)
        : _queue(queue) {
        _mutex = std::make_unique<std::mutex>();
    }

    ~GPUQueue() = default;

    GPUQueue(const GPUQueue& other) = delete;
    GPUQueue(GPUQueue&& other) noexcept = delete;
    GPUQueue& operator=(const GPUQueue& other) = delete;
    GPUQueue& operator=(GPUQueue&& other) noexcept = delete;

    void waitIdle() const noexcept;

    bool submitV1(const VkSubmitInfo& submit_info, VkFence fence = VK_NULL_HANDLE) const noexcept;

    bool submit(const VkSubmitInfo2& submit_info, VkFence fence = VK_NULL_HANDLE) const noexcept;

    bool isShared() const noexcept { return _borrowCount > 0; }

    bool isExclusive() const noexcept { return _borrowCount == 0; }

    VkQueue vkQueue() const noexcept { return _queue; }

  private:
    VkQueue _queue = VK_NULL_HANDLE;
    uint32_t _familyIndex = 0;
    std::unique_ptr<std::mutex> _mutex;
    uint32_t _borrowCount = 0;

    void _acquire() noexcept;

    void _release() noexcept;
};

class GPUQueueHandle final {
  public:
    GPUQueueHandle() = default;

    static GPUQueueHandle borrow(GPUQueue& queue) {
        queue._acquire();
        return GPUQueueHandle(&queue, true);
    }

    static GPUQueueHandle exclusive(GPUQueue& queue) { return GPUQueueHandle(&queue, false); }

    ~GPUQueueHandle() { _release(); }

    GPUQueueHandle(GPUQueueHandle&& other) noexcept
        : _queue(other._queue)
        , _isBorrowed(other._isBorrowed) {
        other._queue = nullptr;
        other._isBorrowed = false;
    }

    GPUQueueHandle& operator=(GPUQueueHandle&& other) noexcept {
        if (this != &other) {
            _release();
            _queue = other._queue;
            _isBorrowed = other._isBorrowed;
            other._queue = nullptr;
            other._isBorrowed = false;
        }
        return *this;
    }

    GPUQueueHandle(const GPUQueueHandle&) = delete;
    GPUQueueHandle& operator=(const GPUQueueHandle&) = delete;

    GPUQueue& getQueue() const noexcept { return *_queue; }

    void release() noexcept { _release(); }

  private:
    GPUQueueHandle(GPUQueue* queue, bool isBorrowed)
        : _queue(queue)
        , _isBorrowed(isBorrowed)
        , _ownerThreadId(std::this_thread::get_id()) {}

    GPUQueue* _queue = nullptr;
    bool _isBorrowed = false;
    std::thread::id _ownerThreadId;

    void _release() noexcept;
};

} // namespace vax::rhi