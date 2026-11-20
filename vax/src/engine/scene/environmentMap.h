#pragma once

#include "buffer.h"
#include "logger.h"
#include "resourceManager.h"
#include "shaderData.h"
#include "textureLoader.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace vax::engine {
class EnvironmentMap final {
  public:
    using EnvironmentMapBuffer = vax::vk::Buffer<EnvironmentMapData>;

    enum class TextureType {
        EnvMap,
        EnvMapIrradiance,
        BRDFLUT,
    };

    struct Descriptor final {
        std::vector<std::pair<TextureType, std::string>> textures;
    };

    EnvironmentMap(const vax::vk::Device& device, vax::vk::ResourceManager& resourceManager) noexcept
        : _device(device)
        , _resourceManager(resourceManager) {
        _textureLoader = std::make_unique<vax::vk::TextureLoader>(device, resourceManager.textureManager());
    };

    ~EnvironmentMap() = default;

    EnvironmentMap(const EnvironmentMap& other) = delete;
    EnvironmentMap(EnvironmentMap&& other) noexcept = delete;
    EnvironmentMap& operator=(const EnvironmentMap& other) = delete;
    EnvironmentMap& operator=(EnvironmentMap&& other) noexcept = delete;

    bool load(const Descriptor& descriptor) noexcept;

    const EnvironmentMapBuffer& environmentMapBuffer() const noexcept { return *_buffer; }

  private:
    vax::Logger _logger = vax::Logger("EnvironmentMap");

    std::reference_wrapper<const vax::vk::Device> _device;
    std::reference_wrapper<const vax::vk::ResourceManager> _resourceManager;

    std::unique_ptr<EnvironmentMapBuffer> _buffer = nullptr;
    std::unique_ptr<vax::vk::TextureLoader> _textureLoader = nullptr;

    EnvironmentMapData _environmentMapData;
};
} // namespace vax::engine