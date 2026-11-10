#pragma once

#include "shaderModuleDescriptor.h"
#include <string>

namespace vax::vk {
struct PipelineDescriptor final {
    enum class Type {
        GRAPHICS,
        COMPUTE,
    };

    Type type;
    std::string name;
    ShaderModuleDescriptor shaderModuleDescriptor;
};

constexpr std::vector<PipelineDescriptor> defaultPipelines() {
    return {
        PipelineDescriptor{
        PipelineDescriptor::Type::COMPUTE, "jfa_init", ShaderModuleDescriptor::Compute("jfa/jfa", "jfa_init")
        },
        PipelineDescriptor{
        PipelineDescriptor::Type::COMPUTE, "jfa_main", ShaderModuleDescriptor::Compute("jfa/jfa", "jfa_main")
        },
        PipelineDescriptor{
        PipelineDescriptor::Type::GRAPHICS, "background", ShaderModuleDescriptor::GraphicsVF("background")
        },
    };
}
} // namespace vax::vk