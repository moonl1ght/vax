#pragma once

#include "logger.h"
#include <descriptorRegistry.h>
#include <device.h>
#include <sampler.h>
#include <shaderResourceHandle.h>

namespace vax::rhi {

class SamplerManager final {
  public:
    enum class GlobalSampler : uint32_t { PBRSampler = 0, CubeMapSampler = 1 };

    struct SamplerResource final {
        Sampler sampler;
        SamplerShaderHandle shaderHandle;
    };

    explicit SamplerManager(const rhi::Device& device, rhi::DescriptorRegistry& descriptorRegistry) noexcept
        : _device(device)
        , _descriptorRegistry(descriptorRegistry) {}

    ~SamplerManager() = default;

    SamplerManager(const SamplerManager&) = delete;
    SamplerManager(SamplerManager&&) noexcept = delete;
    SamplerManager& operator=(const SamplerManager&) = delete;
    SamplerManager& operator=(SamplerManager&&) noexcept = delete;

    bool setup() noexcept;

    const SamplerResource* getGlobalSampler(GlobalSampler sampler) const noexcept;

  private:
    dev::Logger _logger = dev::Logger("SamplerManager");

    std::reference_wrapper<const rhi::Device> _device;
    std::reference_wrapper<rhi::DescriptorRegistry> _descriptorRegistry;

    std::vector<SamplerResource> _samplers;
};

} // namespace vax::rhi