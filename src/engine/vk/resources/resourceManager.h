#pragma once

#include "bufferManager.h"
#include "luna.h"
#include "materialManager.h"
#include "meshManager.h"
#include "ssboManager.h"
#include "textureManager.h"

namespace vax::vk {
class ResourceManager final {
  public:
    explicit ResourceManager(const vax::vk::Device& device)
        : _bufferManager(device)
        , _meshManager(device)
        , _textureManager(device)
        , _materialManager(device)
        , _ssboManager(device) {};

    ResourceManager(const ResourceManager& other) = delete;
    ResourceManager(ResourceManager&& other) noexcept = delete;
    ResourceManager& operator=(const ResourceManager& other) = delete;
    ResourceManager& operator=(ResourceManager&& other) noexcept = delete;

    ~ResourceManager() { _cleanup(); }

    bool setup(uint32_t maxInstances);

    BufferManager& bufferManager() { return _bufferManager; }

    MeshManager& meshManager() { return _meshManager; }

    TextureManager& textureManager() { return _textureManager; }

    MaterialManager& materialManager() { return _materialManager; }

    SSBOManager& ssboManager() { return _ssboManager; }

  private:
    vax::Logger _logger = vax::Logger("ResourceManager");

    BufferManager _bufferManager;
    MeshManager _meshManager;
    TextureManager _textureManager;
    MaterialManager _materialManager;
    SSBOManager _ssboManager;

    void _cleanup();
};
} // namespace vax::vk