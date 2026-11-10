#pragma once

#include "resourceUtils.h"
#include "vertex.h"
#include "device.h"

namespace vax::vk {
class MeshManager;
}

namespace vax::vk {
template <typename VertexType> class MeshObject final {
  public:
    struct GlobalMemoryCursor {
        uint32_t bufferIndex;
        uint32_t offset;
        uint32_t count;
    };

    friend class vax::vk::MeshManager;

    explicit MeshObject(const Device& device, MeshManager& meshManager)
        : _device(device)
        , _meshManager(meshManager) {};

    ~MeshObject() {};

    MeshObject(const MeshObject& other) = delete;
    MeshObject& operator=(const MeshObject& other) = delete;

    MeshObject(MeshObject&& other) noexcept
        : _device(other._device)
        , _meshManager(other._meshManager)
        , _name(other._name)
        , _vertices(std::move(other._vertices))
        , _indices(std::move(other._indices))
        , _id(other._id)
        , _bound(other._bound)
        , _locked(other._locked)
        , _globalMemoryVertexCursor(other._globalMemoryVertexCursor)
        , _globalMemoryIndexCursor(other._globalMemoryIndexCursor)
        , _vertexCount(other._vertexCount)
        , _indexCount(other._indexCount) {
        other._id = NullId;
        other._bound = false;
        other._locked = false;
        other._name.clear();
        other._globalMemoryVertexCursor = std::nullopt;
        other._globalMemoryIndexCursor = std::nullopt;
        other._vertexCount = 0;
        other._indexCount = 0;
    }

    MeshObject& operator=(MeshObject&& other) noexcept {
        if (this != &other) {
            _device = other._device;
            _meshManager = other._meshManager;
            _name = std::move(other._name);
            _vertices = std::move(other._vertices);
            _indices = std::move(other._indices);
            _id = other._id;
            _bound = other._bound;
            _locked = other._locked;
            _globalMemoryVertexCursor = other._globalMemoryVertexCursor;
            _globalMemoryIndexCursor = other._globalMemoryIndexCursor;
            _vertexCount = other._vertexCount;
            _indexCount = other._indexCount;
        }
        return *this;
    }

    MeshId id() const { return _id; }

    const std::vector<VertexType>& vertices() const { return _vertices; }

    const std::vector<uint32_t>& indices() const { return _indices; }

    uint32_t vertexCount() const { return _vertexCount; }

    uint32_t indexCount() const { return _indexCount; }

    void setVertices(std::vector<VertexType> vertices);

    void addVertex(const VertexType& vertex);

    void setIndices(std::vector<uint32_t> indices);

    void addIndex(uint32_t index);

    void setName(const std::string& name) { _name = name; }

    bool bindBuffers();

    void lock();

    void unlock();

    bool isLocked() const { return _locked; }

    bool isBound() const { return _bound; }

    bool flushToGPU();

    const GlobalMemoryCursor& globalMemoryVertexCursor() const { return _globalMemoryVertexCursor.value(); }

    const GlobalMemoryCursor& globalMemoryIndexCursor() const { return _globalMemoryIndexCursor.value(); }

  private:
    vax::Logger _logger = vax::Logger("Mesh");

    std::reference_wrapper<const Device> _device;
    std::reference_wrapper<MeshManager> _meshManager;

    std::string _name;

    MeshId _id = NullId;

    std::vector<VertexType> _vertices;
    std::vector<uint32_t> _indices;

    bool _bound = false;
    bool _locked = false; // if true, the mesh is locked and cannot be modified
    uint32_t _vertexCount = 0;
    uint32_t _indexCount = 0;

    std::optional<GlobalMemoryCursor> _globalMemoryVertexCursor = std::nullopt;
    std::optional<GlobalMemoryCursor> _globalMemoryIndexCursor = std::nullopt;
};

using Mesh = MeshObject<Vertex>;
} // namespace vax::vk