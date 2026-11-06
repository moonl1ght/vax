#pragma once

#include "renderPassDescriptor.h"
#include "shaderModuleDescriptor.h"
#include <string>

namespace vax::vk {
struct PipelineDescriptor {
    enum class Type {
        GRAPHICS,
        COMPUTE,
    };

    Type type;
    std::string name;
    ShaderModuleDescriptor shaderModuleDescriptor;
    std::optional<RenderPassDescriptor> renderPassDescriptor;
};
} // namespace vax::vk