#pragma once

#include "device.h"
#include "logger.h"
#include "resourceUtils.h"
#include "shaderResourceHandle.h"
#include <optional>
#include <string>

namespace vax::rhi {
class CommandBuffer;
class BufferManager;
} // namespace vax::rhi

namespace vax::rhi {

template <typename T> class Buffer final {
  public:
    friend class vax::rhi::BufferManager;

    static std::optional<Buffer<T>> allocateAndFillData(
        const vax::rhi::Device& device,
        std::string name,
        const T* data,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags flags = 0
    );

    static std::optional<Buffer<T>> allocate(
        const vax::rhi::Device& device,
        std::string name,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags flags = 0
    );

    ~Buffer() { cleanup(); }

    Buffer(const vax::rhi::Device& device)
        : _device(device) {};

    Buffer(const Buffer& other) = delete;

    Buffer& operator=(const Buffer& other) = delete;

    Buffer(Buffer&& other)
        : _device(other._device)
        , _vkBuffer(other._vkBuffer)
        , _allocation(other._allocation)
        , _size(other._size)
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
        other._id = NullId;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            cleanup();
            _device = other._device;
            _vkBuffer = other._vkBuffer;
            _allocation = other._allocation;
            _size = other._size;
            _id = other._id;
            _mappedMemory = other._mappedMemory;
            _isMapped = other._isMapped;
            _isPersistentlyMapped = other._isPersistentlyMapped;
            other._vkBuffer = VK_NULL_HANDLE;
            other._allocation = VK_NULL_HANDLE;
            other._mappedMemory = nullptr;
            other._isMapped = false;
            other._size = 0;
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

    void copyBufferCommand(vax::rhi::CommandBuffer& commandBuffer, Buffer& dstBuffer, VkDeviceSize size) const;

    bool isEmpty() const;

    bool isMapped() const { return _isMapped; }

    bool isAllocated() const;

    void cleanup();

    VkBuffer vkBuffer() const { return _vkBuffer; }

    VkDeviceSize size() const { return _size; }

    BufferId id() const { return _id; }

    const std::string& name() const { return _name; }

  private:
    dev::Logger _logger = dev::Logger("Buffer");
    std::reference_wrapper<const vax::rhi::Device> _device;

    std::string _name;
    BufferId _id = NullBufferId;
    VkBuffer _vkBuffer = VK_NULL_HANDLE;
    VmaAllocation _allocation = VK_NULL_HANDLE;
    VkDeviceSize _size = 0;
    bool _isMapped = false;
    bool _isPersistentlyMapped = false;
    T* _mappedMemory = nullptr;

    bool _allocate(VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags);

    void _destroy();
};

using AnyBuffer = Buffer<void>;

template <typename T> using ShaderBufferHandle = std::pair<StructuredBufferShaderHandle, Buffer<T>>;

} // namespace vax::rhi