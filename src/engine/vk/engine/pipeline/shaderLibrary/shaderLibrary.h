#pragma once

#include "logger.h"
#include "reflectionProcessor.h"
#include <optional>
#include <slang-com-ptr.h>
#include <slang.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace vax::vk {

class ShaderLibrary final {
  public:
    enum class Profile {
        sm_5_0,
        sm_6_5,
        sm_6_6,
    };

    struct CompilationResult {
        std::vector<uint32_t> spirv;
        ShaderReflectionData reflectionData;
    };

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

        static EntryPointDescriptor VertexMain() { return Vertex("vertex_main"); }

        static EntryPointDescriptor FragmentMain() { return Fragment("fragment_main"); }

        static EntryPointDescriptor ComputeMain() { return Compute("compute_main"); }

        const std::string name;
        const VkShaderStageFlags stageFlags;
    };

    struct ModuleDescriptor final {

        static ModuleDescriptor VertexFragmentPipeline(const std::string& name) {
            return ModuleDescriptor{name, {EntryPointDescriptor::VertexMain(), EntryPointDescriptor::FragmentMain()}};
        }

        static ModuleDescriptor ComputePipeline(const std::string& name) {
            return ModuleDescriptor{name, {EntryPointDescriptor::ComputeMain()}};
        }

        static ModuleDescriptor VertexPipeline(const std::string& name) {
            return ModuleDescriptor{name, {EntryPointDescriptor::VertexMain()}};
        }

        static ModuleDescriptor FragmentPipeline(const std::string& name) {
            return ModuleDescriptor{name, {EntryPointDescriptor::FragmentMain()}};
        }

        const std::string name;
        std::vector<EntryPointDescriptor> entryPoints;
    };

    ShaderLibrary() noexcept { slang::createGlobalSession(_globalSession.writeRef()); };

    ~ShaderLibrary() noexcept = default;

    ShaderLibrary(const ShaderLibrary&) = delete;
    ShaderLibrary(ShaderLibrary&&) = delete;
    ShaderLibrary& operator=(const ShaderLibrary&) = delete;
    ShaderLibrary& operator=(ShaderLibrary&&) = delete;

    void loadSession(const std::string& shaderDirectory, Profile profile = Profile::sm_6_5) noexcept;

    void loadModule(const ModuleDescriptor& descriptor) noexcept;

  private:
    vax::Logger _logger = vax::Logger("ShaderLibrary");

    std::unique_ptr<ReflectionProcessor> _reflectionProcessor = std::make_unique<ReflectionProcessor>();

    Slang::ComPtr<slang::IGlobalSession> _globalSession;

    Slang::ComPtr<slang::ISession> _session;

    std::optional<CompilationResult>
    _loadEntryPoint(slang::IModule* module, const std::string& name, VkShaderStageFlags stageFlags) noexcept;
};

} // namespace vax::vk