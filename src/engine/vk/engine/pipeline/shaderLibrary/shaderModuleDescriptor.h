#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace vax::vk {
struct EntryPointDescriptor final {

    static EntryPointDescriptor Vertex(const std::string& name) {
        return EntryPointDescriptor{name, VK_SHADER_STAGE_VERTEX_BIT};
    }

    static EntryPointDescriptor Fragment(const std::string& name) {
        return EntryPointDescriptor{name, VK_SHADER_STAGE_FRAGMENT_BIT};
    }

    static EntryPointDescriptor Compute(const std::string& name) {
        return EntryPointDescriptor{name, VK_SHADER_STAGE_COMPUTE_BIT};
    }

    const std::string name;
    const VkShaderStageFlags stageFlags;
};

struct ShaderModuleDescriptor final {

    static ShaderModuleDescriptor Compute(const std::string& name, const std::string& computeName = "compute_main") {
        return ShaderModuleDescriptor{name, {EntryPointDescriptor::Compute(computeName)}};
    }

    static ShaderModuleDescriptor GraphicsVF(
        const std::string& name,
        const std::string& vertexName = "vertex_main",
        const std::string& fragmentName = "fragment_main"
    ) {
        return ShaderModuleDescriptor{
            name, {EntryPointDescriptor::Vertex(vertexName), EntryPointDescriptor::Fragment(fragmentName)}
        };
    }

    static ShaderModuleDescriptor GraphicsV(
        const std::string& name,
        const std::string& vertexName = "vertex_main"
    ) {
        return ShaderModuleDescriptor{
            name, {EntryPointDescriptor::Vertex(vertexName)}
        };
    }

    const std::string name;
    std::vector<EntryPointDescriptor> entryPoints;
};
} // namespace vax::vk