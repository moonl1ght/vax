#pragma once

#include "descriptorSetManager.h"
#include "device.h"
#include "luna.h"
#include "pipeline.h"
#include "pipelineBuilder.h"
#include "pipelineDescriptor.h"
#include "renderPassDescriptor.h"
#include "shaderLibrary.h"
#include "shaderModuleBuilder.h"

namespace vax::vk {
class PipelineManager final {
  public:
    PipelineManager(
        const Device& device, const DescriptorSetManager& descriptorSetManager, ShaderLibrary& shaderLibrary
    )
        : _device(device)
        , _descriptorSetManager(descriptorSetManager)
        , _shaderLibrary(shaderLibrary)
        , _shaderModuleBuilder(device) {};

    ~PipelineManager() {
        for (auto& [name, pipelineLayout] : _pipelineLayouts) {
            vkDestroyPipelineLayout(_device.get().vkDevice, pipelineLayout, nullptr);
        }
    };

    PipelineManager(const PipelineManager&) = delete;
    PipelineManager& operator=(const PipelineManager&) = delete;
    PipelineManager(PipelineManager&& other) = delete;
    PipelineManager& operator=(PipelineManager&& other) = delete;

    void loadShaderLibrary(const std::string& shaderDirectory);

    void unloadShaderLibrary();

    void prebuildPipelines(const std::vector<PipelineDescriptor>& pipelineDescriptors);

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

    std::reference_wrapper<ShaderLibrary> _shaderLibrary;

    std::unique_ptr<DescriptorSetManager> _descriptorSetManagerV2 = nullptr;

    std::unordered_map<std::string, VkPipeline> _pipelinesV2;
    std::unordered_map<std::string, VkPipelineLayout> _pipelineLayouts;

    bool _createComputePipelineLayout(
        PipelineDescriptor& pipelineDescriptor, ShaderLibrary::CompilationResult compilationResult
    );

    bool _createGraphicsPipelineLayout(
        PipelineDescriptor& pipelineDescriptor, ShaderLibrary::CompilationResult compilationResult
    );

    // LEGACY:
    std::reference_wrapper<const Device> _device;
    std::reference_wrapper<const DescriptorSetManager> _descriptorSetManager;

    ShaderModuleBuilder _shaderModuleBuilder;

    std::unordered_map<std::string, Pipeline> _pipelines;

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