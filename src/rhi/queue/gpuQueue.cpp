#include "gpuQueue.h"
#include <vulkan/vulkan_core.h>

using namespace vax;
using namespace vax::rhi;

void GPUQueue::waitIdle() const noexcept {
    if (isShared()) {
        std::lock_guard lock(*_mutex);
        vkQueueWaitIdle(_queue);
    } else {
        vkQueueWaitIdle(_queue);
    }
}

bool GPUQueue::submitV1(const VkSubmitInfo& submit_info, VkFence fence) const noexcept {
    VkResult result;
    if (isShared()) {
        std::lock_guard lock(*_mutex);
        result = vkQueueSubmit(_queue, 1, &submit_info, fence);
    } else {
        result = vkQueueSubmit(_queue, 1, &submit_info, fence);
    }
    return result == VK_SUCCESS;
}

bool GPUQueue::submit(const VkSubmitInfo2& submit_info, VkFence fence) const noexcept {
    VkResult result;
    if (isShared()) {
        std::lock_guard lock(*_mutex);
        result = vkQueueSubmit2(_queue, 1, &submit_info, fence);
    } else {
        result = vkQueueSubmit2(_queue, 1, &submit_info, fence);
    }
    return result == VK_SUCCESS;
}

void GPUQueue::_acquire() noexcept { ++_borrowCount; }

void GPUQueue::_release() noexcept {
    assert(_borrowCount > 0);
    --_borrowCount;
}

void GPUQueueHandle::_release() noexcept {
    assert(std::this_thread::get_id() == _ownerThreadId && "Release must happen on the same thread as borrow!");
    if (_queue && _isBorrowed) {
        _queue->_release();
    }
    _queue = nullptr;
    _isBorrowed = false;
}