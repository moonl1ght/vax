#include "bufferManager.h"

using namespace vax::vk;

void BufferManager::fullCleanup() {
    for (auto& [id, buffer] : _pool) {
        buffer._destroy();
    }
    _pool.clear();
}

std::optional<BufferManager::BufferResource> BufferManager::allocateBuffer(
    std::string name,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags flags
) {
    auto buffer = AnyBuffer(_device.get());
    buffer._name = name;
    buffer._size = size;
    buffer._id = _lastId++;
    if (!buffer._allocate(usage, memoryUsage, flags))
        return std::nullopt;
    auto [it, inserted] = _pool.try_emplace(buffer.id(), std::move(buffer));
    if (!inserted) {
        return std::nullopt;
    }
    buffer._isDetached = false;
    return std::make_pair(it->first, &it->second);
}

std::optional<BufferManager::BufferResource> BufferManager::find(BufferHandle handle) {
    auto it = _pool.find(handle.id());
    if (it == _pool.end())
        return std::nullopt;
    return std::make_pair(handle, &it->second);
}

bool BufferManager::deleteBuffer(BufferHandle handle) {
    auto it = _pool.find(handle.id());
    if (it == _pool.end())
        return false;
    it->second._destroy();
    _pool.erase(it);
    return true;
}

std::optional<AnyBuffer> BufferManager::detach(BufferHandle handle) {
    auto it = _pool.find(handle.id());
    if (it == _pool.end())
        return std::nullopt;
    it->second._detach();
    _pool.erase(it);
    return std::move(it->second);
}
