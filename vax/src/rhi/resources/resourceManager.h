#pragma once

#include "bufferManager.h"
#include "logger.h"
#include "materialManager.h"
#include "meshManager.h"
#include "ssboManager.h"
#include "textureManager.h"
#include <cstdint>

namespace vax::rhi {
class ResourceManager final {
  public:
    explicit ResourceManager(const vax::rhi::Device& device, DescriptorRegistry& descriptorRegistry)
        : _bufferManager(device)
        , _meshManager(device)
        , _textureManager(device, descriptorRegistry)
        , _materialManager(device)
        , _ssboManager(device) {};

    ResourceManager(const ResourceManager& other) = delete;
    ResourceManager(ResourceManager&& other) noexcept = delete;
    ResourceManager& operator=(const ResourceManager& other) = delete;
    ResourceManager& operator=(ResourceManager&& other) noexcept = delete;

    ~ResourceManager() { _cleanup(); }

    bool setup() noexcept;

    bool load(uint32_t maxInstances) noexcept;

    BufferManager& bufferManager() { return _bufferManager; }

    MeshManager& meshManager() { return _meshManager; }

    TextureManager& textureManager() { return _textureManager; }

    const TextureManager& textureManager() const { return _textureManager; }

    MaterialManager& materialManager() { return _materialManager; }

    SSBOManager& ssboManager() { return _ssboManager; }

  private:
    dev::Logger _logger = dev::Logger("ResourceManager");

    BufferManager _bufferManager;
    MeshManager _meshManager;
    TextureManager _textureManager;
    MaterialManager _materialManager;
    SSBOManager _ssboManager;

    void _cleanup();
};
} // namespace vax::rhi