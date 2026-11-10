#include "pipelineBuilder.h"

using namespace vax::vk;

VkPipelineLayout PipelineLayoutBuilder::build(std::string name) noexcept {
    std::vector<VkDescriptorSetLayout> descriptorSetLayoutsVk;
    descriptorSetLayoutsVk.reserve(_descriptorSetLayouts.size());
    for (const auto descriptorSetLayout : _descriptorSetLayouts) {
        descriptorSetLayoutsVk.push_back(descriptorSetLayout->getVkDescriptorSetLayout());
    }
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(descriptorSetLayoutsVk.size()),
        .pSetLayouts = descriptorSetLayoutsVk.data(),
        .pushConstantRangeCount = _pushConstantRange ? 1u : 0u,
        .pPushConstantRanges = _pushConstantRange ? &*_pushConstantRange : nullptr,
    };
    VkPipelineLayout pipelineLayout;
    auto result = vkCreatePipelineLayout(_device.get().vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS) {
        _logger.error("failed to create pipeline layout!");
        return VK_NULL_HANDLE;
    }

    VkDebugUtilsObjectNameInfoEXT nameInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        .objectHandle = reinterpret_cast<size_t>(pipelineLayout),
        .pObjectName = (name + "_pipeline_layout").c_str(),
    };
    vax::vk::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice, &nameInfo);
    return pipelineLayout;
}

// MARK: - ComputePipelineBuilder

std::optional<vax::vk::PipelineV2> ComputePipelineBuilder::build(std::string name) noexcept {
    if (_shaderStageInfo.stage != VK_SHADER_STAGE_COMPUTE_BIT) {
        _logger.error("Compute pipeline requires compute shader stage!");
        return std::nullopt;
    }

    VkComputePipelineCreateInfo computePipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .stage = _shaderStageInfo,
        .layout = _pipelineLayout,
    };

    VkPipeline pipeline;
    auto result = vkCreateComputePipelines(
        _device.get().vkDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline
    );
    if (!VK_CHECK(result)) {
        _logger.error("Failed to create compute pipeline!");
        return std::nullopt;
    }

    std::vector<std::string> descriptorSetNames;
    descriptorSetNames.reserve(_pipelineLayoutKey.setLayouts.size());
    for (size_t i = 0; i < _pipelineLayoutKey.setLayouts.size(); i++) {
        descriptorSetNames.push_back(_pipelineLayoutKey.descriptorSetNames[i]);
    }

    return vax::vk::PipelineV2{
        .pipelineLayoutKey = _pipelineLayoutKey,
        .vkPipeline = pipeline,
        .descriptorSetNames = descriptorSetNames,
    };
}

void ComputePipelineBuilder::setPipelineLayout(
    const PipelineLayoutKey& pipelineLayoutKey, const VkPipelineLayout& pipelineLayout
) noexcept {
    _pipelineLayoutKey = pipelineLayoutKey;
    _pipelineLayout = pipelineLayout;
}

void ComputePipelineBuilder::setShaderModule(const ShaderModule& module, const char* name) noexcept {
    VkPipelineShaderStageCreateInfo shaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = module.vkModule,
        .pName = name,
    };
    _shaderStageInfo = shaderStageInfo;
}

// MARK: - GraphicsPipelineBuilder

VkPipelineLayout GraphicsPipelineBuilder::buildPipelineLayout(std::string name) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(_descriptorSetLayouts.size()),
        .pSetLayouts = _descriptorSetLayouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &_pushConstantRange
    };
    VkPipelineLayout pipelineLayout;
    auto result = vkCreatePipelineLayout(_device.get().vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS) {
        _logger.error("failed to create pipeline layout!");
        return VK_NULL_HANDLE;
    }

    VkDebugUtilsObjectNameInfoEXT nameInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        .objectHandle = reinterpret_cast<size_t>(pipelineLayout),
        .pObjectName = (name + "_pipeline_layout").c_str(),
    };
    vax::vk::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice, &nameInfo);
    return pipelineLayout;
}

std::optional<vax::vk::Pipeline> GraphicsPipelineBuilder::build(std::string name, VkPipelineLayout pipelineLayout) {
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = _topology,
        .primitiveRestartEnable = VK_FALSE,
    };
    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };
    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };
    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };
    VkPipelineDepthStencilStateCreateInfo depthStencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = _depthStencilState.depthTestEnable,
        .depthWriteEnable = _depthStencilState.depthWriteEnable,
        .depthCompareOp = _depthStencilState.depthCompareOp,
        .depthBoundsTestEnable = _depthStencilState.depthBoundsTestEnable,
        .stencilTestEnable = _depthStencilState.stencilTestEnable,
    };
    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_FALSE,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(_colorAttachmentCount, colorBlendAttachment);
    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size()),
        .pAttachments = colorBlendAttachments.data(),
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };
    std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &_bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(_attributeDescriptions.size()),
        .pVertexAttributeDescriptions = _attributeDescriptions.data(),
    };
    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(_shaderStages.size()),
        .pStages = _shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = _renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
    };

    VkPipeline pipeline;
    auto pipelineResult =
        vkCreateGraphicsPipelines(_device.get().vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
    if (pipelineResult != VK_SUCCESS) {
        _logger.error("failed to create graphics pipeline!");
        return std::nullopt;
    }

    VkDebugUtilsObjectNameInfoEXT baseGraphicsPipelineNameInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = VK_OBJECT_TYPE_PIPELINE,
        .objectHandle = reinterpret_cast<size_t>(pipeline),
        .pObjectName = (name + "_pipeline").c_str(),
    };
    vax::vk::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice, &baseGraphicsPipelineNameInfo);
    return vax::vk::Pipeline(_device.get(), name, vax::vk::PipelineType::RENDER, pipelineLayout, pipeline);
}

void GraphicsPipelineBuilder::addShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char* name) {
    VkPipelineShaderStageCreateInfo shaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage,
        .module = module,
        .pName = name,
    };
    _shaderStages.push_back(shaderStageInfo);
}

void GraphicsPipelineBuilder::addVertexInputInfo(
    VkVertexInputBindingDescription bindingDescription,
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions
) {
    _bindingDescription = bindingDescription;
    _attributeDescriptions = std::move(attributeDescriptions);
}

void GraphicsPipelineBuilder::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) {
    _descriptorSetLayouts.push_back(descriptorSetLayout);
}