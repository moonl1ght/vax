#pragma once

#include "buffer.h"
#include "luna.h"
#include "shaderUniforms.h"
#include "textureLoader.h"
#include "resourceManager.h"

namespace vax::engine {
class EnvironmentMap final {
  public:
    using EnvironmentMapBuffer = vax::vk::Buffer<EnvironmentMapData>;

    enum class TextureType {
        EnvMap,
        EnvMapIrradiance,
        BRDFLUT,
    };

    struct Descriptor {
        std::vector<std::pair<TextureType, std::string>> textures;
    };

    EnvironmentMap(const vax::vk::Device& device, vax::vk::ResourceManager& resourceManager)
        : _device(device) {
            _textureLoader = std::make_unique<vax::vk::TextureLoader>(device, resourceManager.textureManager());
        };
    ~EnvironmentMap() = default;

    EnvironmentMap(const EnvironmentMap& other) = delete;
    EnvironmentMap(EnvironmentMap&& other) noexcept
        : _device(other._device)
        , _textureLoader(std::move(other._textureLoader))
        , _environmentMapData(other._environmentMapData)
        , _buffer(std::move(other._buffer)) {
        other._environmentMapData = {};
        other._buffer = nullptr;
    };
    EnvironmentMap& operator=(const EnvironmentMap& other) = delete;
    EnvironmentMap& operator=(EnvironmentMap&& other) noexcept {
        if (this != &other) {
            _textureLoader = std::move(other._textureLoader);
            _device = other._device;
            _environmentMapData = other._environmentMapData;
            _buffer = std::move(other._buffer);
        }
        return *this;
    };

    void load(const Descriptor& descriptor);

    const EnvironmentMapBuffer& environmentMapBuffer() const { return *_buffer; }

  private:
    vax::Logger _logger = vax::Logger("EnvironmentMap");

    std::reference_wrapper<const vax::vk::Device> _device;

    std::unique_ptr<EnvironmentMapBuffer> _buffer = nullptr;
    std::unique_ptr<vax::vk::TextureLoader> _textureLoader = nullptr;

    EnvironmentMapData _environmentMapData;
};
} // namespace vax::engine