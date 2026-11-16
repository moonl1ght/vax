#pragma once

#include "shaderModuleDescriptor.h"
#include <string>

namespace vax::vk {

struct RenderPassDescriptor;

struct DepthStencilState {
    bool depthTestEnable = true;
    bool depthWriteEnable = true;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_GREATER;
    bool depthBoundsTestEnable = false;
    bool stencilTestEnable = false;
};

struct PipelineDescriptor final {
    enum class Type {
        GRAPHICS,
        COMPUTE,
    };

    Type type;
    std::string name;
    ShaderModuleDescriptor shaderModuleDescriptor;
    DepthStencilState depthStencilState = {};
    RenderPassDescriptor* renderPassDescriptor = nullptr;
};

constexpr std::unordered_map<std::string, PipelineDescriptor> defaultPipelines() {
    return {
        {
        "jfa_init",
        PipelineDescriptor{
        PipelineDescriptor::Type::COMPUTE, "jfa_init", ShaderModuleDescriptor::Compute("compute/jfa", "jfa_init")
        },
        },
        {
        "jfa_step",
        PipelineDescriptor{
        PipelineDescriptor::Type::COMPUTE, "jfa_step", ShaderModuleDescriptor::Compute("compute/jfa", "jfa_step")
        },
        },
        {
        "finalblend",
        PipelineDescriptor{
        PipelineDescriptor::Type::GRAPHICS,
        "finalblend",
        ShaderModuleDescriptor::GraphicsVF("finalblend"),
        DepthStencilState{
        .depthWriteEnable = false,
        .depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
        }
        },
        },
        {
        "finalblend_cam",
        PipelineDescriptor{
        PipelineDescriptor::Type::GRAPHICS,
        "finalblend_cam",
        ShaderModuleDescriptor::GraphicsVF("finalblend", "vertex_main", "fragment_cam_main"),
        DepthStencilState{
        .depthWriteEnable = false,
        .depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
        }
        },
        },
        {
        "pbr",
        PipelineDescriptor{
        PipelineDescriptor::Type::GRAPHICS, "pbr", ShaderModuleDescriptor::GraphicsVF("fragments/pbr/pbr")
        },
        },
        {
        "base",
        PipelineDescriptor{
        PipelineDescriptor::Type::GRAPHICS, "base", ShaderModuleDescriptor::GraphicsV("base", "base_vertex_main")
        },
        },
        {
        "simple",
        PipelineDescriptor{
        PipelineDescriptor::Type::GRAPHICS, "simple", ShaderModuleDescriptor::GraphicsVF("fragments/simple")
        },
        },
    };
}
} // namespace vax::vk