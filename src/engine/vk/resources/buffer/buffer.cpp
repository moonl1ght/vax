#include "buffer.h"
#include <cstring>
#include "commandBuffer.h"
#include "shaderUniforms.h"
#include "vertex.h"

using namespace vax::vk;

using namespace vax;

template <typename T>
std::optional<Buffer<T>> Buffer<T>::allocateAndFillData(
    const vax::vk::Device& device,
    std::string name,
    const T* data,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags flags
) {
    auto buffer = Buffer<T>(device);
    buffer._name = name;
    buffer._size = size;
    if (!buffer._allocate(usage, memoryUsage, flags))
        return std::nullopt;
    if (!buffer.fill(data))
        return std::nullopt;
    return buffer;
}

template <typename T>
std::optional<Buffer<T>> Buffer<T>::allocate(
    const vax::vk::Device& device,
    std::string name,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags flags
) {
    auto buffer = Buffer<T>(device);
    buffer._name = name;
    buffer._size = size;
    if (!buffer._allocate(usage, memoryUsage, flags))
        return std::nullopt;
    return buffer;
}

template <typename T> void Buffer<T>::cleanup() {
    if (isDetached())
        _destroy();
}

template <typename T> void Buffer<T>::_detach() {
    _isDetached = true;
    _id = NullId;
}

template <typename T> void Buffer<T>::_destroy() {
    unmap();
    if (_vkBuffer != VK_NULL_HANDLE && _allocation != VK_NULL_HANDLE) {
        vmaDestroyBuffer(_device.get().allocator(), _vkBuffer, _allocation);
        _vkBuffer = VK_NULL_HANDLE;
        _allocation = VK_NULL_HANDLE;
    }
    _mappedMemory = nullptr;
    _isMapped = false;
    _isDetached = true;
    _id = NullId;
}

template <typename T>
bool Buffer<T>::reload(
    const T* data,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags flags
) {
    cleanup();
    _size = size;
    if (!_allocate(usage, memoryUsage, flags))
        return false;
    if (!fill(data))
        return false;
    return true;
}

template <typename T>
bool Buffer<T>::load(
    const T* data,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags flags
) {
    if (isAllocated()) {
        return false;
    }

    _size = size;
    if (!_allocate(usage, memoryUsage, flags))
        return false;
    if (!fill(static_cast<const void*>(data), size))
        return false;
    return true;
}

template <typename T> bool Buffer<T>::fill(const void* fillData, size_t bytesFill, size_t bytesOffset) {
    if (isEmpty() || !isAllocated() || fillData == nullptr || bytesFill == 0) {
        return false;
    }

    map();
    memcpy(reinterpret_cast<char*>(_mappedMemory) + bytesOffset, fillData, bytesFill);
    unmap();
    return true;
}

template <typename T> bool Buffer<T>::fill(const void* fillData) {
    return fill(fillData, static_cast<size_t>(_size), 0);
}

template <typename T>
void vax::vk::Buffer<T>::copyBufferCommand(
    vax::vk::CommandBuffer& commandBuffer, Buffer<T>& dstBuffer, VkDeviceSize size
) const {
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer.vkCommandBuffer, _vkBuffer, dstBuffer._vkBuffer, 1, &copyRegion);
}

template <typename T> bool Buffer<T>::isEmpty() const { return _size == 0; }

template <typename T> bool Buffer<T>::isAllocated() const {
    return _vkBuffer != VK_NULL_HANDLE && _allocation != VK_NULL_HANDLE;
}

template <typename T>
bool Buffer<T>::_allocate(VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags) {
    if (isAllocated()) {
        return false;
    }

    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = _size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo allocInfo{
        .flags = flags,
        .usage = memoryUsage,
    };

    VmaAllocationInfo resultAllocInfo{};
    if (vmaCreateBuffer(_device.get().allocator(), &bufferInfo, &allocInfo, &_vkBuffer, &_allocation, &resultAllocInfo) !=
        VK_SUCCESS) {
        _logger.error("failed to create VMA buffer!");
        return false;
    }

    if (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
        _mappedMemory = static_cast<T*>(resultAllocInfo.pMappedData);
        _isMapped = true;
        _isPersistentlyMapped = true;
    }

    if (!_name.empty()) {
        vmaSetAllocationName(_device.get().allocator(), _allocation, _name.c_str());

        VkDebugUtilsObjectNameInfoEXT nameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = reinterpret_cast<uint64_t>(_vkBuffer),
            .pObjectName = _name.c_str(),
        };
        vax::vk::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice(), &nameInfo);
    }
    return true;
}

template <typename T> void Buffer<T>::map() {
    if (isMapped())
        return;
    void* mappedData = nullptr;
    vmaMapMemory(_device.get().allocator(), _allocation, &mappedData);
    _mappedMemory = static_cast<T*>(mappedData);
    _isMapped = true;
}

template <typename T> void Buffer<T>::unmap() {
    if (!isMapped())
        return;
    if (_isPersistentlyMapped) {
        return;
    }
    vmaUnmapMemory(_device.get().allocator(), _allocation);
    _isMapped = false;
    _mappedMemory = nullptr;
}

template <typename T> std::optional<T*> Buffer<T>::mappedMemory() const {
    if (!isMapped())
        return std::nullopt;
    return _mappedMemory;
}

template class vax::vk::Buffer<void>;
template class vax::vk::Buffer<uint32_t>;
template class vax::vk::Buffer<vax::vk::Vertex>;
template class vax::vk::Buffer<PBRMaterial>;
template class vax::vk::Buffer<InstanceData>;
template class vax::vk::Buffer<EnvironmentMapData>;
template class vax::vk::Buffer<VkDrawIndexedIndirectCommand>;
template class vax::vk::Buffer<PerDrawData>;
