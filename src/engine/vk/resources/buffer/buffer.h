#pragma once

#include "device.h"
#include "vulkanCommon.h"
#include "logger.h"
#include <optional>
#include <string>
#include "resourceUtils.h"

namespace vax::vk {
class CommandBuffer;
class BufferManager;
} // namespace vax::vk

namespace vax::vk {
template <typename T> class Buffer final {
  public:
    friend class vax::vk::BufferManager;

    static std::optional<Buffer<T>> allocateAndFillData(
        const vax::vk::Device& device,
        std::string name,
        const T* data,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags flags = 0
    );

    static std::optional<Buffer<T>> allocate(
        const vax::vk::Device& device,
        std::string name,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags flags = 0
    );

    ~Buffer() { cleanup(); }

    Buffer(const vax::vk::Device& device)
        : _device(device) {};

    Buffer(const Buffer& other) = delete;

    Buffer& operator=(const Buffer& other) = delete;

    Buffer(Buffer&& other)
        : _device(other._device)
        , _vkBuffer(other._vkBuffer)
        , _allocation(other._allocation)
        , _size(other._size)
        , _isDetached(other._isDetached)
        , _id(other._id)
        , _mappedMemory(other._mappedMemory)
        , _isMapped(other._isMapped)
        , _isPersistentlyMapped(other._isPersistentlyMapped) {
        other._vkBuffer = VK_NULL_HANDLE;
        other._allocation = VK_NULL_HANDLE;
        other._mappedMemory = nullptr;
        other._isMapped = false;
        other._isPersistentlyMapped = false;
        other._size = 0;
        other._isDetached = true;
        other._id = NullId;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            cleanup();
            _device = other._device;
            _vkBuffer = other._vkBuffer;
            _allocation = other._allocation;
            _size = other._size;
            _isDetached = other._isDetached;
            _id = other._id;
            _mappedMemory = other._mappedMemory;
            _isMapped = other._isMapped;
            _isPersistentlyMapped = other._isPersistentlyMapped;
            other._vkBuffer = VK_NULL_HANDLE;
            other._allocation = VK_NULL_HANDLE;
            other._mappedMemory = nullptr;
            other._isMapped = false;
            other._size = 0;
            other._isDetached = true;
            other._id = NullId;
        }
        return *this;
    }

    bool load(
        const T* data,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags flags = 0
    );

    bool reload(
        const T* data,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags flags = 0
    );

    bool fill(const void* fillData, size_t bytesFill, size_t bytesOffset = 0);

    bool fill(const void* fillData);

    void map();

    void unmap();

    std::optional<T*> mappedMemory() const;

    void copyBufferCommand(vax::vk::CommandBuffer& commandBuffer, Buffer& dstBuffer, VkDeviceSize size) const;

    bool isEmpty() const;

    bool isMapped() const { return _isMapped; }

    bool isAllocated() const;

    void cleanup();

    VkBuffer vkBuffer() const { return _vkBuffer; }

    VkDeviceSize size() const { return _size; }

    BufferId id() const { return _id; }

    bool isDetached() const { return _isDetached; }

    const std::string& name() const { return _name; }

  private:
    vax::Logger _logger = vax::Logger("Buffer");
    std::reference_wrapper<const vax::vk::Device> _device;

    std::string _name;
    BufferId _id = NullBufferId;
    VkBuffer _vkBuffer = VK_NULL_HANDLE;
    VmaAllocation _allocation = VK_NULL_HANDLE;
    VkDeviceSize _size = 0;
    bool _isMapped = false;
    bool _isPersistentlyMapped = false;
    T* _mappedMemory = nullptr;
    bool _isDetached = true;

    bool _allocate(VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags);

    void _destroy();

    void _detach();
};

using AnyBuffer= Buffer<void>;
} // namespace vax::vk