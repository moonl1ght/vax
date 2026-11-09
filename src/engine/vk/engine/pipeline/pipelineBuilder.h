#pragma once

#include "descriptorSetLayout.h"
#include "device.h"
#include "pipeline.h"
#include "pipelinev2.h"
#include "shaderModule.h"
#include <optional>
#include <vulkan/vulkan_core.h>

namespace vax::vk {

struct DepthStencilState {
    bool depthTestEnable = true;
    bool depthWriteEnable = true;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_GREATER;
    bool depthBoundsTestEnable = false;
    bool stencilTestEnable = false;
};

// MARK: - ComputePipelineBuilder

class PipelineLayoutBuilder final {
  public:
    explicit PipelineLayoutBuilder(const vax::vk::Device& device) noexcept
        : _device(device) {};

    ~PipelineLayoutBuilder() = default;

    VkPipelineLayout build(std::string name) noexcept;

    void setDescriptorSetLayouts(const std::vector<DescriptorSetLayout*>& descriptorSetLayouts) noexcept {
        _descriptorSetLayouts = descriptorSetLayouts;
    }

    void setPushConstantRange(const VkPushConstantRange& pushConstantRange) noexcept {
        _pushConstantRange = pushConstantRange;
    }

  private:
    vax::Logger _logger = vax::Logger("PipelineLayoutBuilder");
    std::reference_wrapper<const vax::vk::Device> _device;
    std::vector<DescriptorSetLayout*> _descriptorSetLayouts;
    std::optional<VkPushConstantRange> _pushConstantRange;
};

class ComputePipelineBuilder final {
  public:
    explicit ComputePipelineBuilder(const vax::vk::Device& device) noexcept
        : _device(device) {};

    ~ComputePipelineBuilder() = default;

    std::optional<vax::vk::PipelineV2> build(std::string name) noexcept;

    void setPipelineLayout(const VkPipelineLayout& pipelineLayout) noexcept;

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

    std::optional<vax::vk::Pipeline> build(std::string name, VkPipelineLayout pipelineLayout);
    VkPipelineLayout buildPipelineLayout(std::string name);

    void addShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char* name);
    void addVertexInputInfo(
        VkVertexInputBindingDescription bindingDescription,
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions
    );
    void setInputAssembly(VkPrimitiveTopology topology) { _topology = topology; }
    void addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
    void setPushConstantRange(VkPushConstantRange pushConstantRange) { _pushConstantRange = pushConstantRange; }
    void setRenderPass(VkRenderPass renderPass) { _renderPass = renderPass; }
    void setDepthStencilState(DepthStencilState depthStencilState) { _depthStencilState = depthStencilState; }
    void setColorAttachmentCount(uint32_t colorAttachmentCount) { _colorAttachmentCount = colorAttachmentCount; }

  private:
    vax::Logger _logger = vax::Logger("GraphicsPipelineBuilder");
    std::reference_wrapper<const vax::vk::Device> _device;
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
    VkVertexInputBindingDescription _bindingDescription = {};
    std::vector<VkVertexInputAttributeDescription> _attributeDescriptions;
    std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;
    VkPrimitiveTopology _topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPushConstantRange _pushConstantRange = {};
    VkRenderPass _renderPass = VK_NULL_HANDLE;
    DepthStencilState _depthStencilState = {};
    uint32_t _colorAttachmentCount = 1;
};
} // namespace vax::vk