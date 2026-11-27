#include "ssboManager.h"

using namespace vax::rhi;

void SSBOManager::cleanup() {
    for (auto& buf : _buffers) {
        if (buf) {
            _descriptorRegistry.get().releaseStorageBuffer(buf->first);
            buf->second.cleanup();
            buf = nullptr;
        }
    }
}

bool SSBOManager::setup(uint32_t maxInstances) {
    _maxInstances = maxInstances;
    for (uint32_t i = 0; i < vax::rhi::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto allocation = InstanceBuffer::allocate(
            _device.get(),
            "ssbo_instance_buffer",
            sizeof(InstanceData) * maxInstances,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        );
        if (!allocation.has_value())
            return false;
        allocation->map();
        auto handle = _descriptorRegistry.get().registerStorageBuffer(*allocation);
        if (!handle.has_value())
            return false;
        _buffers[i] = std::make_unique<InstanceBufferResource>(*handle, std::move(*allocation));
    }
    return true;
}

bool SSBOManager::updateInstance(uint32_t frameIndex, uint32_t index, const InstanceData& instance) {
    if (index >= _maxInstances)
        return false;
    auto& buf = _buffers[frameIndex];
    if (!buf)
        return false;
    auto mappedMemory = buf->second.mappedMemory();
    if (!mappedMemory.has_value())
        return false;
    InstanceData* instancePtr = mappedMemory.value();
    instancePtr[index] = instance;
    return true;
}
