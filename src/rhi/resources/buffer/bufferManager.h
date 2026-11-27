#pragma once

#include "buffer.h"
#include "device.h"
#include "logger.h"
#include "resourceHandle.h"
#include "vulkanCommon.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace vax::rhi {
class BufferManager final {
  public:
    using BufferResource = std::pair<BufferHandle, AnyBuffer*>;

    explicit BufferManager(const Device& device)
        : _device(device) {};

    ~BufferManager() { fullCleanup(); }

    BufferManager(const BufferManager& other) = delete;
    BufferManager(BufferManager&& other) noexcept = delete;
    BufferManager& operator=(const BufferManager& other) = delete;
    BufferManager& operator=(BufferManager&& other) noexcept = delete;

    void fullCleanup();

    std::optional<BufferResource> allocateBuffer(
        std::string name,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags flags = 0
    );

    std::optional<BufferResource> find(BufferHandle handle);

    bool deleteBuffer(BufferHandle handle);

  private:
    dev::Logger _logger = dev::Logger("BufferManager");

    std::reference_wrapper<const Device> _device;
    // TODO: change to vector + use generation for stability
    // maybe vector of vectors of buffers?
    std::unordered_map<BufferId, AnyBuffer> _pool;
    BufferId _lastId = 0;
};
} // namespace vax::rhi