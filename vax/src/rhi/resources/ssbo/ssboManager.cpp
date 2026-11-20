#include "ssboManager.h"

using namespace vax::vk;

void SSBOManager::cleanup() {
    for (auto& buf : _buffers) {
        if (buf)
            buf->cleanup();
    }
}

bool SSBOManager::setup(uint32_t maxInstances) {
    _maxInstances = maxInstances;
    for (uint32_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
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
        _buffers[i] = std::make_unique<InstanceBuffer>(std::move(*allocation));
        _buffers[i]->map();
    }
    return true;
}

bool SSBOManager::updateInstance(uint32_t frameIndex, uint32_t index, const InstanceData& instance) {
    if (index >= _maxInstances)
        return false;
    auto& buf = _buffers[frameIndex];
    if (!buf)
        return false;
    auto mappedMemory = buf->mappedMemory();
    if (!mappedMemory.has_value())
        return false;
    InstanceData* instancePtr = mappedMemory.value();
    instancePtr[index] = instance;
    return true;
}
