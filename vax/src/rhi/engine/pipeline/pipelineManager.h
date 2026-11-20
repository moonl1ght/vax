#pragma once

#include "descriptorManager.h"
#include "device.h"
#include "pipeline.h"
#include "pipelineDescriptor.h"
#include "shaderLibrary.h"
#include <vector>

namespace vax::rhi {
class PipelineManager final {
  public:
    PipelineManager(const Device& device)
        : _device(device) {};

    ~PipelineManager() { _destroy(); };

    PipelineManager(const PipelineManager&) = delete;
    PipelineManager& operator=(const PipelineManager&) = delete;
    PipelineManager(PipelineManager&& other) = delete;
    PipelineManager& operator=(PipelineManager&& other) = delete;

    bool setup() noexcept;

    bool load(const std::string& shaderDirectory) noexcept;

    void unload() noexcept;

    bool prebuildPipelines(const std::vector<PipelineDescriptor>& pipelineDescriptors) noexcept;

    std::optional<Pipeline> getPipeline(const std::string& pipelineName) const noexcept {
        auto it = _pipelines.find(pipelineName);
        if (it == _pipelines.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    DescriptorRegistry& descriptorRegistry() const noexcept { return _descriptorManager->descriptorRegistry(); }

    VkPipelineLayout pipelineLayout() const noexcept { return _pipelineLayout; }

    VkDescriptorSet bindlessDescriptorSet() const noexcept { return _descriptorManager->descriptorSet(); }

    VkDescriptorSet uboDescriptorSet(uint32_t frameIndex) const noexcept {
        return _descriptorManager->uboDescriptorSet(frameIndex);
    }

  private:
    vax::Logger _logger = vax::Logger("PipelineManager");

    std::reference_wrapper<const Device> _device;

    bool _isLoaded = false;

    std::unique_ptr<ShaderLibrary> _shaderLibrary = nullptr;
    std::unique_ptr<DescriptorManager> _descriptorManager = nullptr;

    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

    std::unordered_map<std::string, Pipeline> _pipelines;

    void _destroy() noexcept;
};
} // namespace vax::rhi