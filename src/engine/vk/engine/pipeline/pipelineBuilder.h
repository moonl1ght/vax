#pragma once

#include "device.h"
#include "pipelineDescriptor.h"
#include "renderPassDescriptor.h"
#include "shaderModule.h"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vax::vk {

// MARK: - ComputePipelineBuilder

class PipelineLayoutBuilder final {
  public:
    explicit PipelineLayoutBuilder(const vax::vk::Device& device, uint32_t pushConstantSize = 128) noexcept
        : _device(device)
        , _pushConstantSize(pushConstantSize) {};

    ~PipelineLayoutBuilder() = default;

    VkPipelineLayout build(std::string name) const noexcept;

    void addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) noexcept {
        _descriptorSetLayouts.push_back(descriptorSetLayout);
    }

  private:
    vax::Logger _logger = vax::Logger("PipelineLayoutBuilder");
    const uint32_t _pushConstantSize;
    std::reference_wrapper<const vax::vk::Device> _device;
    std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;
};

class ComputePipelineBuilder final {
  public:
    explicit ComputePipelineBuilder(const vax::vk::Device& device) noexcept
        : _device(device) {};

    ~ComputePipelineBuilder() = default;

    std::optional<VkPipeline> build(std::string name) noexcept;

    void setPipelineLayout(const VkPipelineLayout& pipelineLayout) noexcept { _pipelineLayout = pipelineLayout; }

    void setShaderModule(const ShaderModule& module, const char* name) noexcept;

  private:
    vax::Logger _logger = vax::Logger("ComputePipelineBuilder");
    std::reference_wrapper<const vax::vk::Device> _device;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo _shaderStageInfo = {};
};

// MARK: - GraphicsPipelineBuilder

class GraphicsPipelineBuilder final {
  public:
    explicit GraphicsPipelineBuilder(const vax::vk::Device& device)
        : _device(device) {};

    std::optional<VkPipeline> build(std::string name, VkPipelineLayout pipelineLayout) noexcept;

    void addShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char* name) noexcept;
    void setInputAssembly(VkPrimitiveTopology topology) noexcept { _topology = topology; }
    void setRenderPass(RenderPassDescriptor* renderPassDescriptor) noexcept {
        _renderPassDescriptor = renderPassDescriptor;
    }
    void setDepthStencilState(DepthStencilState depthStencilState) noexcept { _depthStencilState = depthStencilState; }

  private:
    vax::Logger _logger = vax::Logger("GraphicsPipelineBuilder");
    std::reference_wrapper<const vax::vk::Device> _device;
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
    VkPrimitiveTopology _topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    RenderPassDescriptor* _renderPassDescriptor = nullptr;
    DepthStencilState _depthStencilState = {};
};
} // namespace vax::vk