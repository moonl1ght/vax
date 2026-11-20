#include "mesh.h"
#include "meshManager.h"
#include "vertex.h"
#include <cstdint>

using namespace vax::vk;

template <typename VertexType> void vax::vk::MeshObject<VertexType>::setVertices(std::vector<VertexType> vertices) {
    if (!_locked) {
        _vertices = std::move(vertices);
        _vertexCount = _vertices.size();
    }
}

template <typename VertexType> void vax::vk::MeshObject<VertexType>::addVertex(const VertexType& vertex) {
    if (!_locked) {
        _vertices.push_back(vertex);
        ++_vertexCount;
    }
}

template <typename VertexType> void vax::vk::MeshObject<VertexType>::setIndices(std::vector<uint32_t> indices) {
    if (!_locked) {
        _indices = std::move(indices);
        _indexCount = _indices.size();
    }
}

template <typename VertexType> void vax::vk::MeshObject<VertexType>::addIndex(uint32_t index) {
    if (!_locked) {
        _indices.push_back(index);
        ++_indexCount;
    }
}

template <typename VertexType> void vax::vk::MeshObject<VertexType>::lock() { _locked = true; }

template <typename VertexType> void vax::vk::MeshObject<VertexType>::unlock() { _locked = false; }

template <typename VertexType> bool vax::vk::MeshObject<VertexType>::bindBuffers() {
    if (!_locked) {
        return false;
    }
    if (_bound) {
        return true;
    }
    auto result = _meshManager.get()._tryToBindMemory(*this, false);
    if (!result) {
        return false;
    }
    _globalMemoryVertexCursor = {result.value().vertexBufferIndex, result.value().vertexOffset, _vertexCount};
    _globalMemoryIndexCursor = {result.value().indexBufferIndex, result.value().indexOffset, _indexCount};
    _bound = true;
    return true;
}

template <typename VertexType> bool vax::vk::MeshObject<VertexType>::flushToGPU() {
    if (!_bound) {
        return false;
    }

    if (!_globalMemoryVertexCursor.has_value() || !_globalMemoryIndexCursor.has_value()) {
        return false;
    }


    uint32_t vertexBufferSize = _globalMemoryVertexCursor->count * sizeof(VertexType);
    uint32_t vertexOffset = _globalMemoryVertexCursor->offset * sizeof(VertexType);
    uint32_t indexBufferSize = _globalMemoryIndexCursor->count * sizeof(uint32_t);
    uint32_t indexOffset = _globalMemoryIndexCursor->offset * sizeof(uint32_t);

    _meshManager.get()._globalVertexBuffers[_globalMemoryVertexCursor->bufferIndex]->fill(
        static_cast<const void*>(_vertices.data()), vertexBufferSize, vertexOffset
    );
    _meshManager.get()._globalIndexBuffers[_globalMemoryIndexCursor->bufferIndex]->fill(
        static_cast<const void*>(_indices.data()), indexBufferSize, indexOffset
    );

    _vertices.clear();
    _indices.clear();

    return true;
}

template class vax::vk::MeshObject<vax::vk::Vertex>;