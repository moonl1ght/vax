#pragma once

#include "shaderModuleDescriptor.h"
#include <string>

namespace vax::vk {

struct RenderPassDescriptor;

struct PipelineDescriptor final {
    enum class Type {
        GRAPHICS,
        COMPUTE,
    };

    Type type;
    std::string name;
    ShaderModuleDescriptor shaderModuleDescriptor;
    const RenderPassDescriptor* renderPassDescriptor = nullptr;
};

constexpr std::unordered_map<std::string, PipelineDescriptor> defaultPipelines() {
    return {
        {
        "jfa_init",
        PipelineDescriptor{
        PipelineDescriptor::Type::COMPUTE, "jfa_init", ShaderModuleDescriptor::Compute("jfa/jfa", "jfa_init")
        },
        },
        {
        "jfa_step",
        PipelineDescriptor{
        PipelineDescriptor::Type::COMPUTE, "jfa_step", ShaderModuleDescriptor::Compute("jfa/jfa", "jfa_step")
        },
        },
        {
        "background",
        PipelineDescriptor{
        PipelineDescriptor::Type::GRAPHICS, "background", ShaderModuleDescriptor::GraphicsVF("background")
        },
        },
    };
}
} // namespace vax::vk