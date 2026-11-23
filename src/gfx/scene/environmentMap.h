#pragma once

#include "buffer.h"
#include "logger.h"
#include "resourceManager.h"
#include "shaderData.h"
#include "textureGpuUploader.h"
#include "textureLibrary.h"
#include "textureLoader.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace vax::gfx {
class EnvironmentMap final {
  public:
    using EnvironmentMapBuffer = vax::rhi::Buffer<EnvironmentMapData>;

    enum class TextureType {
        EnvMap,
        EnvMapIrradiance,
        BRDFLUT,
    };

    struct Descriptor final {
        std::vector<std::pair<TextureType, std::string>> textures;
    };

    EnvironmentMap(const vax::rhi::Device& device) noexcept
        : _device(device) {};

    ~EnvironmentMap() = default;

    EnvironmentMap(const EnvironmentMap& other) = delete;
    EnvironmentMap(EnvironmentMap&& other) noexcept = delete;
    EnvironmentMap& operator=(const EnvironmentMap& other) = delete;
    EnvironmentMap& operator=(EnvironmentMap&& other) noexcept = delete;

    bool load(
        const Descriptor& descriptor,
        rhi::ResourceManager& resourceManager,
        rhi::TextureLoader& textureLoader,
        vax::rhi::TextureGpuUploader& textureGpuUploader
    ) noexcept;

    const EnvironmentMapBuffer& environmentMapBuffer() const noexcept { return *_buffer; }

  private:
    dev::Logger _logger = dev::Logger("EnvironmentMap");

    std::reference_wrapper<const vax::rhi::Device> _device;

    std::unique_ptr<EnvironmentMapBuffer> _buffer = nullptr;

    EnvironmentMapData _environmentMapData;
};
} // namespace vax::gfx