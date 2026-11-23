#pragma once

#include "bufferManager.h"
#include "logger.h"
#include "materialManager.h"
#include "meshManager.h"
#include "samplerManager.h"
#include "ssboManager.h"
#include "textureFactory.h"
#include "textureLibrary.h"
#include <cstdint>

namespace vax::rhi {
class ResourceManager final {
  public:
    explicit ResourceManager(const vax::rhi::Device& device, DescriptorRegistry& descriptorRegistry)
        : _bufferManager(device)
        , _meshManager(device)
        , _samplerManager(device, descriptorRegistry)
        , _textureLibrary()
        , _materialManager(device)
        , _ssboManager(device) {};

    ResourceManager(const ResourceManager& other) = delete;
    ResourceManager(ResourceManager&& other) noexcept = delete;
    ResourceManager& operator=(const ResourceManager& other) = delete;
    ResourceManager& operator=(ResourceManager&& other) noexcept = delete;

    ~ResourceManager() { _cleanup(); }

    bool setup() noexcept;

    bool load(uint32_t maxInstances) noexcept;

    SamplerManager& samplerManager() { return _samplerManager; }

    const SamplerManager& samplerManager() const { return _samplerManager; }

    TextureLibrary& textureLibrary() { return _textureLibrary; }

    const TextureLibrary& textureLibrary() const { return _textureLibrary; }

    BufferManager& bufferManager() { return _bufferManager; }

    MeshManager& meshManager() { return _meshManager; }

    MaterialManager& materialManager() { return _materialManager; }

    SSBOManager& ssboManager() { return _ssboManager; }

  private:
    dev::Logger _logger = dev::Logger("ResourceManager");

    BufferManager _bufferManager;
    MeshManager _meshManager;
    SamplerManager _samplerManager;
    TextureLibrary _textureLibrary;
    MaterialManager _materialManager;
    SSBOManager _ssboManager;

    void _cleanup();
};
} // namespace vax::rhi