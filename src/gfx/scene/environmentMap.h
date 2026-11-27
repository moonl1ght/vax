#pragma once

#include "buffer.h"
#include "logger.h"
#include "samplerManager.h"
#include "shaderData.h"
#include "textureLibrary.h"
#include "textureGpuUploader.h"
#include "textureLoader.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace vax::gfx {
class EnvironmentMap final {
  public:
    using EnvironmentMapBuffer = rhi::Buffer<EnvironmentMapData>;
    using EnvironmentMapBufferResoruce = std::pair<StructuredBufferShaderHandle, EnvironmentMapBuffer>;

    enum class TextureType {
        EnvMap,
        EnvMapIrradiance,
        BRDFLUT,
    };

    struct Descriptor final {
        std::vector<std::pair<TextureType, std::string>> textures;
    };

    EnvironmentMap(const vax::rhi::Device& device, rhi::DescriptorRegistry& descriptorRegistry) noexcept
        : _device(device)
        , _descriptorRegistry(descriptorRegistry) {};

    ~EnvironmentMap() = default;

    EnvironmentMap(const EnvironmentMap& other) = delete;
    EnvironmentMap(EnvironmentMap&& other) noexcept = delete;
    EnvironmentMap& operator=(const EnvironmentMap& other) = delete;
    EnvironmentMap& operator=(EnvironmentMap&& other) noexcept = delete;

    void cleanup() noexcept;

    bool load(
        const Descriptor& descriptor,
        const rhi::SamplerManager& samplerManager,
        rhi::TextureLibrary& textureLibrary,
        rhi::TextureLoader& textureLoader,
        vax::rhi::TextureGpuUploader& textureGpuUploader
    ) noexcept;

    const EnvironmentMapBufferResoruce& environmentMapBufferResource() const noexcept { return *_bufferResource; }

  private:
    dev::Logger _logger = dev::Logger("EnvironmentMap");

    std::reference_wrapper<const vax::rhi::Device> _device;
    std::reference_wrapper<rhi::DescriptorRegistry> _descriptorRegistry;

    std::unique_ptr<EnvironmentMapBufferResoruce> _bufferResource = nullptr;

    EnvironmentMapData _environmentMapData;
};
} // namespace vax::gfx