#pragma once

#include "logger.h"
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

    struct ShaderReflectionData {
        std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings;
        std::vector<VkPushConstantRange> pushConstants;
    };

    struct CompilationResult {
        std::vector<uint32_t> spirv;
        ShaderReflectionData reflectionData;
    };

    ShaderLibrary() noexcept { slang::createGlobalSession(_globalSession.writeRef()); };

    ~ShaderLibrary() noexcept = default;

    ShaderLibrary(const ShaderLibrary&) = delete;
    ShaderLibrary(ShaderLibrary&&) = delete;
    ShaderLibrary& operator=(const ShaderLibrary&) = delete;
    ShaderLibrary& operator=(ShaderLibrary&&) = delete;

    void loadSession(const std::string& shaderDirectory, Profile profile = Profile::sm_6_5) noexcept;

    void loadModule(const std::string& name) noexcept;

  private:
    vax::Logger _logger = vax::Logger("ShaderLibrary");

    Slang::ComPtr<slang::IGlobalSession> _globalSession;

    Slang::ComPtr<slang::ISession> _session;

    std::optional<CompilationResult> _loadEntryPoint(
        Slang::ComPtr<slang::IModule> module, const std::string& name, VkShaderStageFlags stageFlags
    ) noexcept;
};

} // namespace vax::vk