#include "meshManager.h"
#include "luna.h"

using namespace vax::vk;

void MeshManager::loadGlobalBuffers(size_t size) {
    VkDeviceSize targetSizeInBytes = size * 1024ULL * 1024;
    VkDeviceSize vertexStride = sizeof(Vertex);

    uint32_t maxVertexCount = targetSizeInBytes / vertexStride;
    uint32_t maxIndexCount = maxVertexCount * 3;
    VkDeviceSize alignedBufferSize = maxVertexCount * vertexStride;
    VkDeviceSize indexBufferSize = maxIndexCount * sizeof(uint32_t);
    auto buffer = VertexBuffer::allocate(
        _device.get(),
        "global_vertex_buffer",
        alignedBufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    );
    if (!buffer) {
        _logger.error("Failed to allocate global vertex buffer");
        return;
    }
    _globalVertexBuffers.push_back(std::make_unique<VertexBuffer>(std::move(*buffer)));
    auto indexBuffer = IndexBuffer::allocate(
        _device.get(),
        "global_index_buffer",
        indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    );
    if (!indexBuffer) {
        _logger.error("Failed to allocate global index buffer");
        return;
    }
    _globalIndexBuffers.push_back(std::make_unique<IndexBuffer>(std::move(*indexBuffer)));
    auto vertexBufferDescriptor = BufferDescriptor{
        .usedMemoryChunks = {},
        .freeMemoryChunks = {{.offset = 0, .count = maxVertexCount}},
        .maxNumberOfElements = maxVertexCount,
        .usedElements = 0
    };
    auto indexBufferDescriptor = BufferDescriptor{
        .usedMemoryChunks = {},
        .freeMemoryChunks = {{.offset = 0, .count = maxIndexCount}},
        .maxNumberOfElements = maxIndexCount,
        .usedElements = 0
    };
    _vertexBufferDescriptors.push_back(vertexBufferDescriptor);
    _indexBufferDescriptors.push_back(indexBufferDescriptor);
}

void MeshManager::fullCleanup() {
    _pool.clear();
    _globalVertexBuffers.clear();
    _globalIndexBuffers.clear();
    _vertexBufferDescriptors.clear();
    _indexBufferDescriptors.clear();
    _lastId = 0;
}

std::optional<MeshManager::MeshResource> MeshManager::createEmptyMesh() {
    auto mesh = Mesh(_device.get(), *this);
    mesh._id = _lastId++;
    auto [it, inserted] = _pool.try_emplace(mesh.id(), std::move(mesh));
    if (!inserted) {
        return std::nullopt;
    }
    return std::make_pair(it->first, &it->second);
}

std::optional<MeshManager::MeshResource> MeshManager::find(MeshHandle handle) {
    auto it = _pool.find(handle.id());
    if (it == _pool.end())
        return std::nullopt;
    return std::make_pair(handle, &it->second);
}

// bool MeshManager::deleteMesh(MeshHandle handle) {
//     auto it = _pool.find(handle.id());
//     if (it == _pool.end())
//         return false;
//     it->second._destroy();
//     _pool.erase(it);
//     return true;
// }

std::optional<MeshManager::MeshResource>
MeshManager::createMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices) {
    auto mesh = Mesh(_device.get(), *this);
    mesh._id = _lastId++;
    mesh.setVertices(vertices);
    mesh.setIndices(indices);
    mesh.lock();
    if (!mesh.bindBuffers()) {
        return std::nullopt;
    }
    if (!mesh.flushToGPU()) {
        return std::nullopt;
    }
    auto [it, inserted] = _pool.try_emplace(mesh.id(), std::move(mesh));
    if (!inserted) {
        return std::nullopt;
    }
    return std::make_pair(it->first, &it->second);
}

// TODO: allocate new pool if needed, for now only one pool is supported
template <typename VertexType>
std::optional<MeshManager::BindMemoryResult>
MeshManager::_tryToBindMemory(const MeshObject<VertexType>& mesh, bool allocateNewPool) {
    auto vertexSlot = _tryToFindFreeMemory(_vertexBufferDescriptors, mesh.vertexCount());
    if (!vertexSlot) {
        _logger.error("Failed to find free global vertex memory for mesh");
        return std::nullopt;
    }
    auto indexSlot = _tryToFindFreeMemory(_indexBufferDescriptors, mesh.indexCount());
    if (!indexSlot) {
        _logger.error("Failed to find free global index memory for mesh");
        return std::nullopt;
    }

    _commitMemory(_vertexBufferDescriptors, *vertexSlot, mesh.vertexCount());
    _commitMemory(_indexBufferDescriptors, *indexSlot, mesh.indexCount());

    return BindMemoryResult{
        .vertexBufferIndex = vertexSlot->bufferIndex,
        .indexBufferIndex = indexSlot->bufferIndex,
        .vertexOffset = vertexSlot->offset,
        .indexOffset = indexSlot->offset
    };
}

std::optional<MeshManager::FreeMemorySlot>
MeshManager::_tryToFindFreeMemory(const std::vector<BufferDescriptor>& bufferDescriptors, uint32_t count) const {
    for (uint32_t bufferIndex = 0; bufferIndex < bufferDescriptors.size(); ++bufferIndex) {
        const auto& freeMemoryChunks = bufferDescriptors[bufferIndex].freeMemoryChunks;
        for (uint32_t chunkIndex = 0; chunkIndex < freeMemoryChunks.size(); ++chunkIndex) {
            if (freeMemoryChunks[chunkIndex].count >= count) {
                return FreeMemorySlot{bufferIndex, chunkIndex, freeMemoryChunks[chunkIndex].offset};
            }
        }
    }
    return std::nullopt;
}

void MeshManager::_commitMemory(
    std::vector<BufferDescriptor>& bufferDescriptors, const FreeMemorySlot& slot, uint32_t count
) {
    if (count == 0) {
        return;
    }
    auto& bufferDescriptor = bufferDescriptors[slot.bufferIndex];
    auto& freeMemoryChunk = bufferDescriptor.freeMemoryChunks[slot.chunkIndex];
    bufferDescriptor.usedMemoryChunks.push_back({freeMemoryChunk.offset, count});
    bufferDescriptor.usedElements += count;
    freeMemoryChunk.offset += count;
    freeMemoryChunk.count -= count;
    if (freeMemoryChunk.count == 0) {
        bufferDescriptor.freeMemoryChunks.erase(bufferDescriptor.freeMemoryChunks.begin() + slot.chunkIndex);
    }
}

template std::optional<MeshManager::BindMemoryResult>
MeshManager::_tryToBindMemory<Vertex>(const MeshObject<Vertex>&, bool);