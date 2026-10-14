#include "pipelineManager.h"
#include "descriptorSetManager.h"
#include "shaderModuleBuilder.h"
#include "shaderUniforms.h"
#include "vkEngine.h"
#include <optional>

using namespace vax::vk;
using namespace vax;

bool PipelineManager::setup(
    const RenderPassDescriptor& renderPassDescriptor,
    const RenderPassDescriptor& postProcessRenderPassDescriptor,
    const RenderPassDescriptor& shadowRenderPassDescriptor
) {
    if (!_createBackgroundPipelineLayout(PipelineLayoutName::BACKGROUND)) {
        _logger.error("Failed to create background pipeline layout!");
        return false;
    }
    if (!_createBasePipelineLayout(PipelineLayoutName::BASE)) {
        _logger.error("Failed to create base pipeline layout!");
        return false;
    }
    if (!_createFinalBlendPipelineLayout(PipelineLayoutName::FINAL_BLEND)) {
        _logger.error("Failed to create final blend pipeline layout!");
        return false;
    }
    if (!_createAgentCameraFBPipelineLayout(PipelineLayoutName::AGENT_CAMERA_FB)) {
        _logger.error("Failed to create agent camera FB pipeline layout!");
        return false;
    }
    if (!_createPBRPipeline(renderPassDescriptor)) {
        _logger.error("Failed to create PBR pipeline!");
        return false;
    }
    if (!_createAgentCameraPipeline(renderPassDescriptor)) {
        _logger.error("Failed to create agent camera pipeline!");
        return false;
    }
    if (!_createBasePipeline(renderPassDescriptor)) {
        _logger.error("Failed to create base pipeline!");
        return false;
    }
    if (!_createBackgroundPipeline(renderPassDescriptor)) {
        _logger.error("Failed to create background pipeline!");
        return false;
    }
    if (!_createFinalBlendPipeline(postProcessRenderPassDescriptor)) {
        _logger.error("Failed to create final blend pipeline!");
        return false;
    }
    if (!_createAgentCameraFBPipeline(postProcessRenderPassDescriptor)) {
        _logger.error("Failed to create agent camera FB pipeline!");
        return false;
    }
    if (!_createShadowPipeline(shadowRenderPassDescriptor)) {
        _logger.error("Failed to create shadow pipeline!");
        return false;
    }
    return true;
}

const Pipeline* PipelineManager::getPipeline(PipelineName pipelineName) const {
    auto name = Pipeline::pipelineNameToString(pipelineName);
    auto it = _pipelines.find(name);
    if (it == _pipelines.end()) {
        _logger.error("Pipeline not found!");
        return nullptr;
    }
    return &it->second;
}

bool PipelineManager::_createBackgroundPipeline(const RenderPassDescriptor& renderPassDescriptor) {
    return _createPipeline(
        renderPassDescriptor,
        SRC_PATH("engine/shaders/out/background.vert.spv"),
        SRC_PATH("engine/shaders/out/background.frag.spv"),
        PipelineName::BACKGROUND,
        PipelineLayoutName::BACKGROUND,
        [](GraphicsPipelineBuilder& pipelineBuilder) {
            pipelineBuilder.setDepthStencilState({
                .depthWriteEnable = false,
                .depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
            });
            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            auto attributeDescriptionsVector = std::vector<VkVertexInputAttributeDescription>(
                attributeDescriptions.begin(), attributeDescriptions.end()
            );
            pipelineBuilder.addVertexInputInfo(bindingDescription, attributeDescriptionsVector);
        }
    );
}

bool PipelineManager::_createShadowPipeline(const RenderPassDescriptor& renderPassDescriptor) {
    return _createPipeline(
        renderPassDescriptor,
        SRC_PATH("engine/shaders/out/base.vert.spv"),
        "",
        PipelineName::SHADOW,
        PipelineLayoutName::BASE,
        [](GraphicsPipelineBuilder& pipelineBuilder) {
            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            auto attributeDescriptionsVector = std::vector<VkVertexInputAttributeDescription>(
                attributeDescriptions.begin(), attributeDescriptions.end()
            );
            pipelineBuilder.addVertexInputInfo(bindingDescription, attributeDescriptionsVector);
        }
    );
}

bool PipelineManager::_createFinalBlendPipeline(const RenderPassDescriptor& renderPassDescriptor) {
    return _createPipeline(
        renderPassDescriptor,
        SRC_PATH("engine/shaders/out/background.vert.spv"),
        SRC_PATH("engine/shaders/out/finalblend.frag.spv"),
        PipelineName::FINAL_BLEND,
        PipelineLayoutName::FINAL_BLEND,
        [](GraphicsPipelineBuilder& pipelineBuilder) {
            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            auto attributeDescriptionsVector = std::vector<VkVertexInputAttributeDescription>(
                attributeDescriptions.begin(), attributeDescriptions.end()
            );
            pipelineBuilder.addVertexInputInfo(bindingDescription, attributeDescriptionsVector);
            pipelineBuilder.setDepthStencilState({
                .depthWriteEnable = false,
                .depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
            });
        }
    );
}

bool PipelineManager::_createAgentCameraFBPipeline(const RenderPassDescriptor& renderPassDescriptor) {
    return _createPipeline(
        renderPassDescriptor,
        SRC_PATH("engine/shaders/out/background.vert.spv"),
        SRC_PATH("engine/shaders/out/finalblend.cam.frag.spv"),
        PipelineName::AGENT_CAMERA_FB,
        PipelineLayoutName::AGENT_CAMERA_FB,
        [](GraphicsPipelineBuilder& pipelineBuilder) {
            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            auto attributeDescriptionsVector = std::vector<VkVertexInputAttributeDescription>(
                attributeDescriptions.begin(), attributeDescriptions.end()
            );
            pipelineBuilder.addVertexInputInfo(bindingDescription, attributeDescriptionsVector);
            pipelineBuilder.setDepthStencilState({
                .depthWriteEnable = false,
                .depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
            });
        }
    );
}

bool PipelineManager::_createPBRPipeline(const RenderPassDescriptor& renderPassDescriptor) {
    return _createPipeline(
        renderPassDescriptor,
        SRC_PATH("engine/shaders/out/base.vert.spv"),
        SRC_PATH("engine/shaders/out/pbr.frag.spv"),
        PipelineName::PBR,
        PipelineLayoutName::BASE,
        [](GraphicsPipelineBuilder& pipelineBuilder) {
            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            auto attributeDescriptionsVector = std::vector<VkVertexInputAttributeDescription>(
                attributeDescriptions.begin(), attributeDescriptions.end()
            );
            pipelineBuilder.addVertexInputInfo(bindingDescription, attributeDescriptionsVector);
        }
    );
}

bool PipelineManager::_createAgentCameraPipeline(const RenderPassDescriptor& renderPassDescriptor) {
    return _createPipeline(
        renderPassDescriptor,
        SRC_PATH("engine/shaders/out/base.vert.spv"),
        SRC_PATH("engine/shaders/out/pbr.frag.spv"),
        PipelineName::AGENT_CAMERA,
        PipelineLayoutName::BASE,
        [](GraphicsPipelineBuilder& pipelineBuilder) {
            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            auto attributeDescriptionsVector = std::vector<VkVertexInputAttributeDescription>(
                attributeDescriptions.begin(), attributeDescriptions.end()
            );
            pipelineBuilder.addVertexInputInfo(bindingDescription, attributeDescriptionsVector);
        }
    );
}

bool PipelineManager::_createBasePipeline(const RenderPassDescriptor& renderPassDescriptor) {
    return _createPipeline(
        renderPassDescriptor,
        SRC_PATH("engine/shaders/out/base.vert.spv"),
        SRC_PATH("engine/shaders/out/base.frag.spv"),
        PipelineName::BASE,
        PipelineLayoutName::BASE,
        [](GraphicsPipelineBuilder& pipelineBuilder) {
            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            auto attributeDescriptionsVector = std::vector<VkVertexInputAttributeDescription>(
                attributeDescriptions.begin(), attributeDescriptions.end()
            );
            pipelineBuilder.addVertexInputInfo(bindingDescription, attributeDescriptionsVector);
        }
    );
}

bool vax::vk::PipelineManager::_createBackgroundPipelineLayout(vax::vk::PipelineLayoutName pipelineLayoutName) {
    auto pipelineBuilder = vax::vk::GraphicsPipelineBuilder(_device.get());
    auto name = vax::vk::Pipeline::pipelineLayoutNameToString(pipelineLayoutName);
    auto pipelineLayout = pipelineBuilder.buildPipelineLayout(name);
    if (!pipelineLayout) {
        _logger.error("Failed to create background pipeline layout!");
        return false;
    }
    _pipelineLayouts.emplace(name, pipelineLayout);
    return true;
}

bool vax::vk::PipelineManager::_createFinalBlendPipelineLayout(vax::vk::PipelineLayoutName pipelineLayoutName) {
    auto pipelineBuilder = vax::vk::GraphicsPipelineBuilder(_device.get());
    auto name = vax::vk::Pipeline::pipelineLayoutNameToString(pipelineLayoutName);
    auto finalBlendDescriptorSetLayout =
        _descriptorSetManager.get().getDescriptorSetLayout(DescriptorSetLayoutName::FINAL_BLEND);
    auto maskDescriptorSetLayout =
        _descriptorSetManager.get().getDescriptorSetLayout(DescriptorSetLayoutName::SINGLE_STORAGE_IMAGE);
    auto perFrameDescriptorSetLayout =
        _descriptorSetManager.get().getDescriptorSetLayout(DescriptorSetLayoutName::PER_FRAME);
    if (!finalBlendDescriptorSetLayout || !maskDescriptorSetLayout || !perFrameDescriptorSetLayout) {
        _logger.error("Failed to get final blend descriptor set layout!");
        return false;
    }
    pipelineBuilder.addDescriptorSetLayout(finalBlendDescriptorSetLayout->getVkDescriptorSetLayout());
    pipelineBuilder.addDescriptorSetLayout(maskDescriptorSetLayout->getVkDescriptorSetLayout());
    pipelineBuilder.addDescriptorSetLayout(perFrameDescriptorSetLayout->getVkDescriptorSetLayout());
    auto pipelineLayout = pipelineBuilder.buildPipelineLayout(name);
    if (!pipelineLayout) {
        _logger.error("Failed to create final blend pipeline layout!");
        return false;
    }
    _pipelineLayouts.emplace(name, pipelineLayout);
    return true;
}

bool vax::vk::PipelineManager::_createAgentCameraFBPipelineLayout(vax::vk::PipelineLayoutName pipelineLayoutName) {
    auto pipelineBuilder = vax::vk::GraphicsPipelineBuilder(_device.get());
    auto name = vax::vk::Pipeline::pipelineLayoutNameToString(pipelineLayoutName);
    auto finalBlendCamSampledDescriptorSetLayout = _descriptorSetManager.get().getDescriptorSetLayout(
        DescriptorSetLayoutName::FINAL_BLEND_SIMPLE
    );
    if (!finalBlendCamSampledDescriptorSetLayout) {
        _logger.error("Failed to get agent camera FB descriptor set layout!");
        return false;
    }
    pipelineBuilder.addDescriptorSetLayout(finalBlendCamSampledDescriptorSetLayout->getVkDescriptorSetLayout());
    auto pipelineLayout = pipelineBuilder.buildPipelineLayout(name);
    if (!pipelineLayout) {
        _logger.error("Failed to create agent camera FB pipeline layout!");
        return false;
    }
    _pipelineLayouts.emplace(name, pipelineLayout);
    return true;
}

bool vax::vk::PipelineManager::_createBasePipelineLayout(vax::vk::PipelineLayoutName pipelineLayoutName) {
    auto pipelineBuilder = vax::vk::GraphicsPipelineBuilder(_device.get());
    auto name = vax::vk::Pipeline::pipelineLayoutNameToString(pipelineLayoutName);
    auto globalDescriptorSetLayout =
        _descriptorSetManager.get().getDescriptorSetLayout(DescriptorSetLayoutName::GLOBAL);
    if (!globalDescriptorSetLayout) {
        _logger.error("Failed to get global descriptor set layout!");
        return false;
    }
    pipelineBuilder.addDescriptorSetLayout(globalDescriptorSetLayout->getVkDescriptorSetLayout());
    auto perFrameDescriptorSetLayout =
        _descriptorSetManager.get().getDescriptorSetLayout(DescriptorSetLayoutName::PER_FRAME);
    if (!perFrameDescriptorSetLayout) {
        _logger.error("Failed to get per frame descriptor set layout!");
        return false;
    }
    pipelineBuilder.addDescriptorSetLayout(perFrameDescriptorSetLayout->getVkDescriptorSetLayout());
    auto perDrawDescriptorSetLayout =
        _descriptorSetManager.get().getDescriptorSetLayout(DescriptorSetLayoutName::PER_DRAW);
    if (!perDrawDescriptorSetLayout) {
        _logger.error("Failed to get per draw descriptor set layout!");
        return false;
    }
    pipelineBuilder.addDescriptorSetLayout(perDrawDescriptorSetLayout->getVkDescriptorSetLayout());
    pipelineBuilder.setPushConstantRange({
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(GlobalPushConstants),
    });
    auto pipelineLayout = pipelineBuilder.buildPipelineLayout(name);
    if (!pipelineLayout) {
        _logger.error("Failed to create base pipeline layout!");
        return false;
    }
    _pipelineLayouts.emplace(name, pipelineLayout);
    return true;
}

VkPipelineLayout vax::vk::PipelineManager::getPipelineLayout(vax::vk::PipelineLayoutName pipelineLayoutName) const {
    auto name = vax::vk::Pipeline::pipelineLayoutNameToString(pipelineLayoutName);
    auto it = _pipelineLayouts.find(name);
    if (it == _pipelineLayouts.end()) {
        _logger.error("Pipeline layout not found!");
        return VK_NULL_HANDLE;
    }
    return it->second;
}

bool PipelineManager::_createPipeline(
    const RenderPassDescriptor& renderPassDescriptor,
    std::string vertShaderPath,
    std::string fragShaderPath,
    PipelineName pipelineName,
    PipelineLayoutName pipelineLayoutName,
    std::function<void(GraphicsPipelineBuilder&)> builder
) {
    bool hasFragShader = !fragShaderPath.empty();
    bool hasVertShader = !vertShaderPath.empty();
    std::optional<VkShaderModule> vertShaderModule = std::nullopt;
    if (hasVertShader) {
        vertShaderModule = _shaderModuleBuilder.build(vertShaderPath);
        if (!vertShaderModule) {
            _logger.error("Failed to build vertex shader module!");
            return false;
        }
    }

    std::optional<VkShaderModule> fragShaderModule = std::nullopt;
    if (hasFragShader) {
        fragShaderModule = _shaderModuleBuilder.build(fragShaderPath);
        if (!fragShaderModule) {
            _logger.error("Failed to build fragment shader module!");
            return false;
        }
    }

    auto pipelineBuilder = vax::vk::GraphicsPipelineBuilder(_device.get());
    pipelineBuilder.setRenderPass(renderPassDescriptor.getVkRenderPass());
    pipelineBuilder.setColorAttachmentCount(renderPassDescriptor.colorAttachmentCount);
    if (hasVertShader) {
        pipelineBuilder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule.value(), "main");
    }
    if (hasFragShader) {
        pipelineBuilder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule.value(), "main");
    }
    builder(pipelineBuilder);
    auto name = vax::vk::Pipeline::pipelineNameToString(pipelineName);
    auto pipelineLayoutNameString = vax::vk::Pipeline::pipelineLayoutNameToString(pipelineLayoutName);
    auto it = _pipelineLayouts.find(pipelineLayoutNameString);
    if (it == _pipelineLayouts.end()) {
        _logger.error("Pipeline layout not found!");
        return false;
    }
    auto pipeline = pipelineBuilder.build(name, it->second);
    if (!pipeline) {
        _logger.error("Failed to create base pipeline!");
        return false;
    }
    _pipelines.emplace(name, std::move(*pipeline));
    if (hasFragShader) {
        vkDestroyShaderModule(_device.get().vkDevice, fragShaderModule.value(), nullptr);
    }
    if (hasVertShader) {
        vkDestroyShaderModule(_device.get().vkDevice, vertShaderModule.value(), nullptr);
    }
    return true;
}