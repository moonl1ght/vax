#include "samplerManager.h"

using namespace vax::rhi;

bool SamplerManager::setup() noexcept {
    auto globalSampler = Sampler::linear(_device.get());
    auto globalCubeMapSampler = Sampler::cube(_device.get());
    if (!globalSampler.has_value() || !globalCubeMapSampler.has_value()) {
        _logger.error("Failed to create global samplers");
        return false;
    }
    auto linearSamplerHandle = _descriptorRegistry.get().registerSampler(globalSampler.value().vkSampler());
    auto cubeMapSamplerHandle = _descriptorRegistry.get().registerSampler(globalCubeMapSampler.value().vkSampler());
    if (!linearSamplerHandle.has_value() || !cubeMapSamplerHandle.has_value()) {
        _logger.error("Failed to register global samplers");
        return false;
    }
    SamplerResource linearSamplerResource{
        .sampler = std::move(globalSampler.value()),
        .shaderHandle = *linearSamplerHandle,
    };
    _samplers.emplace_back(std::move(linearSamplerResource));
    SamplerResource cubeMapSamplerResource{
        .sampler = std::move(globalCubeMapSampler.value()),
        .shaderHandle = *cubeMapSamplerHandle,
    };
    _samplers.emplace_back(std::move(cubeMapSamplerResource));
    return true;
}

const SamplerManager::SamplerResource* SamplerManager::getGlobalSampler(GlobalSampler sampler) const noexcept {
    auto index = static_cast<uint32_t>(sampler);
    if (index >= _samplers.size()) {
        _logger.error("Global sampler index out of bounds");
        return nullptr;
    }
    return &_samplers[index];
}