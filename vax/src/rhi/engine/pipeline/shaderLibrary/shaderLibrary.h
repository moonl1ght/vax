#pragma once

#include "logger.h"
#include "reflectionProcessor.h"
#include "shaderModuleDescriptor.h"
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

    struct ShaderFunction final {
        std::string name;
        VkShaderStageFlags stageFlags;
        std::vector<uint32_t> spirv;
    };

    struct PipelineModule final {
        std::string name;
        std::vector<ShaderFunction> functions;
        ShaderReflectionData reflectionData;
    };

    ShaderLibrary() noexcept { slang::createGlobalSession(_globalSession.writeRef()); };

    ~ShaderLibrary() noexcept {
        endSession();
        _globalSession = nullptr;
    };

    ShaderLibrary(const ShaderLibrary&) = delete;
    ShaderLibrary(ShaderLibrary&&) = delete;
    ShaderLibrary& operator=(const ShaderLibrary&) = delete;
    ShaderLibrary& operator=(ShaderLibrary&&) = delete;

    void startSession(const std::string& shaderDirectory, Profile profile = Profile::sm_6_5) noexcept;

    void endSession() noexcept;

    std::optional<PipelineModule> loadPipelineModule(const ShaderModuleDescriptor& descriptor) noexcept;

  private:
    vax::Logger _logger = vax::Logger("ShaderLibrary");

    std::unique_ptr<const ReflectionProcessor> _reflectionProcessor = std::make_unique<ReflectionProcessor>();

    Slang::ComPtr<slang::IGlobalSession> _globalSession;

    Slang::ComPtr<slang::ISession> _session;

    slang::IModule* _loadModule(const std::string& name) noexcept;
};

} // namespace vax::vk