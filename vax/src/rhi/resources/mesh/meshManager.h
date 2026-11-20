#pragma once

#include "buffer.h"
#include "device.h"
#include "mesh.h"
#include "resourceHandle.h"
#include "vertex.h"

namespace vax::vk {
class MeshManager final {
  public:
    using VertexBuffer = Buffer<Vertex>;
    using IndexBuffer = Buffer<uint32_t>;
    using MeshResource = std::pair<MeshHandle, Mesh*>;

    template <typename VertexType> friend class MeshObject;

    explicit MeshManager(const Device& device)
        : _device(device) {};

    ~MeshManager() { fullCleanup(); }

    MeshManager(const MeshManager& other) = delete;
    MeshManager(MeshManager&& other) noexcept = delete;
    MeshManager& operator=(const MeshManager& other) = delete;
    MeshManager& operator=(MeshManager&& other) noexcept = delete;

    /// size in MB
    void loadGlobalBuffers(size_t size);

    void fullCleanup();

    std::optional<MeshResource> createEmptyMesh();

    std::optional<MeshResource> createMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

    std::optional<MeshResource> find(MeshHandle handle);

    const VertexBuffer& globalVertexBuffer(size_t bufferIndex) const { return *_globalVertexBuffers[bufferIndex]; }

    const IndexBuffer& globalIndexBuffer(size_t bufferIndex) const { return *_globalIndexBuffers[bufferIndex]; }

    // TODO: implement mesh deletion with freeing memory
    // bool deleteMesh(MeshHandle handle);

  private:
    struct ChunkInfo {
        uint32_t offset;
        uint32_t count;
    };

    struct BindMemoryResult {
        uint32_t vertexBufferIndex;
        uint32_t indexBufferIndex;
        uint32_t vertexOffset;
        uint32_t indexOffset;
    };

    struct FreeMemorySlot {
        uint32_t bufferIndex;
        uint32_t chunkIndex;
        uint32_t offset;
    };

    struct BufferDescriptor {
        std::vector<ChunkInfo> usedMemoryChunks;
        std::vector<ChunkInfo> freeMemoryChunks;
        uint32_t maxNumberOfElements;
        uint32_t usedElements;
    };

    vax::Logger _logger = vax::Logger("MeshManager");

    std::reference_wrapper<const Device> _device;

    std::vector<std::unique_ptr<VertexBuffer>> _globalVertexBuffers;
    std::vector<std::unique_ptr<IndexBuffer>> _globalIndexBuffers;

    std::vector<BufferDescriptor> _vertexBufferDescriptors;
    std::vector<BufferDescriptor> _indexBufferDescriptors;

    std::unordered_map<MeshId, Mesh> _pool;
    MeshId _lastId = 0;

    template <typename VertexType>
    std::optional<BindMemoryResult> _tryToBindMemory(const MeshObject<VertexType>& mesh, bool allocateNewPool);

    std::optional<FreeMemorySlot>
    _tryToFindFreeMemory(const std::vector<BufferDescriptor>& bufferDescriptors, uint32_t count) const;

    void _commitMemory(std::vector<BufferDescriptor>& bufferDescriptors, const FreeMemorySlot& slot, uint32_t count);
};
} // namespace vax::vk