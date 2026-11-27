#include "bufferManager.h"

using namespace vax::rhi;

void BufferManager::fullCleanup() { _pool.clear(); }

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