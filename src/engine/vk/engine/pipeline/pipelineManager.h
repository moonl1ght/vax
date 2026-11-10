#pragma once

#include "descriptorSetManager.h"
#include "device.h"
#include "luna.h"
#include "pipeline.h"
#include "pipelineBuilder.h"
#include "pipelineDescriptor.h"
#include "pipelinev2.h"
#include "renderPassDescriptor.h"
#include "shaderLibrary.h"
#include "shaderModule.h"
#include <vector>

namespace vax::vk {
class PipelineManager final {
  public:
    PipelineManager(const Device& device, const DescriptorSetManager& descriptorSetManager)
        : _device(device)
        , _descriptorSetManager(descriptorSetManager)
        , _shaderModuleBuilder(device) {
        _descriptorSetManagerV2 = std::make_unique<DescriptorSetManager>(device, MAX_FRAMES_IN_FLIGHT);
    };

    ~PipelineManager() {
        _destroyPipelinesV2();
        _destroyPipelineLayoutsV2();
        for (auto& [name, pipelineLayout] : _pipelineLayouts) {
            vkDestroyPipelineLayout(_device.get().vkDevice, pipelineLayout, nullptr);
        }
    };

    PipelineManager(const PipelineManager&) = delete;
    PipelineManager& operator=(const PipelineManager&) = delete;
    PipelineManager(PipelineManager&& other) = delete;
    PipelineManager& operator=(PipelineManager&& other) = delete;

    void load(const std::string& shaderDirectory) noexcept;

    void unload() noexcept;

    void prebuildPipelines(const std::vector<PipelineDescriptor>& pipelineDescriptors) noexcept;

    std::optional<PipelineHandle> getPipeline(const std::string& pipelineName) const noexcept;

    // Legacy

    bool setup(
        const RenderPassDescriptor& renderPassDescriptor,
        const RenderPassDescriptor& postProcessRenderPassDescriptor,
        const RenderPassDescriptor& shadowRenderPassDescriptor
    );

    const Pipeline* getPipeline(PipelineName pipelineName) const;

    VkPipelineLayout getPipelineLayout(PipelineLayoutName pipelineLayoutName) const;

  private:
    vax::Logger _logger = vax::Logger("PipelineManager");

    bool _isLoaded = false;

    std::unique_ptr<ShaderLibrary> _shaderLibrary = std::make_unique<ShaderLibrary>();
    std::unique_ptr<DescriptorSetManager> _descriptorSetManagerV2 = nullptr;

    std::unordered_map<std::string, PipelineV2> _pipelinesV2;
    std::unordered_map<PipelineLayoutKey, VkPipelineLayout> _pipelineLayoutsV2;

    void _destroyPipelinesV2() noexcept;

    void _destroyPipelineLayoutsV2() noexcept;

    void _buildComputePipeline(const PipelineDescriptor& pipelineDescriptor);

    void _buildGraphicsPipeline(const PipelineDescriptor& pipelineDescriptor);

    // LEGACY:
    std::reference_wrapper<const Device> _device;
    std::reference_wrapper<const DescriptorSetManager> _descriptorSetManager;

    ShaderModuleBuilder _shaderModuleBuilder;

    std::unordered_map<std::string, Pipeline> _pipelines;
    std::unordered_map<std::string, VkPipelineLayout> _pipelineLayouts;

    bool _createBackgroundPipelineLayout(PipelineLayoutName pipelineLayoutName);
    bool _createBasePipelineLayout(PipelineLayoutName pipelineLayoutName);
    bool _createFinalBlendPipelineLayout(PipelineLayoutName pipelineLayoutName);
    bool _createAgentCameraFBPipelineLayout(PipelineLayoutName pipelineLayoutName);

    bool _createPBRPipeline(const RenderPassDescriptor& renderPassDescriptor);
    bool _createAgentCameraPipeline(const RenderPassDescriptor& renderPassDescriptor);
    bool _createAgentCameraFBPipeline(const RenderPassDescriptor& renderPassDescriptor);
    bool _createBackgroundPipeline(const RenderPassDescriptor& renderPassDescriptor);
    bool _createBasePipeline(const RenderPassDescriptor& renderPassDescriptor);
    bool _createFinalBlendPipeline(const RenderPassDescriptor& renderPassDescriptor);
    bool _createShadowPipeline(const RenderPassDescriptor& renderPassDescriptor);

    bool _createPipeline(
        const RenderPassDescriptor& renderPassDescriptor,
        std::string vertShaderPath,
        std::string fragShaderPath,
        PipelineName pipelineName,
        PipelineLayoutName pipelineLayoutName,
        std::function<void(GraphicsPipelineBuilder&)> builder
    );
};
} // namespace vax::vk