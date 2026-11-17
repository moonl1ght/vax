#pragma once

#include "device.h"
#include "logger.h"
#include "poolDescriptor.h"
#include "resourceHandle.h"
#include "sampler.h"
#include "texture.h"
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace vax::vk {
class TextureFactory;
} // namespace vax::vk

namespace vax::vk {
class TextureManager final {
  public:
    using TextureResource = std::pair<TextureHandle, Texture*>;
    using SamplerResource = std::pair<SamplerHandle, Sampler*>;

    explicit TextureManager(const vk::Device& device)
        : _device(device) {
        _globalSamplers.reserve(MAX_SAMPLERS);
    };

    ~TextureManager() { fullCleanup(); }

    TextureManager(const TextureManager& other) = delete;
    TextureManager(TextureManager&& other) noexcept = delete;
    TextureManager& operator=(const TextureManager& other) = delete;
    TextureManager& operator=(TextureManager&& other) noexcept = delete;

    void fullCleanup();

    bool setup();

    TextureFactory createTextureFactory() const;

    std::optional<TextureResource> find(TextureHandle handle);

    bool deleteTexture(TextureHandle handle);

    std::optional<Texture> detach(TextureHandle handle);

    std::optional<TextureResource> attach(Texture&& texture);

    std::optional<SamplerResource> getGlobalSampler(GlobalSampler sampler);

    std::vector<const Texture*> allTextures() const;

    void loadAllTextures(vax::vk::CommandBuffer& commandBuffer);

    void resetAllTexturesStagingBuffers();

  private:
    vax::Logger _logger = vax::Logger("TextureManager");

    std::reference_wrapper<const vk::Device> _device;
    // TODO: change to vector + use generation for stability
    // maybe vector of vectors of buffers?
    std::unordered_map<TextureId, Texture> _pool;
    std::vector<Sampler> _globalSamplers;
    TextureId _lastId = 0;
};
} // namespace vax::vk