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
        ShaderReflectionData reflectionData;
    };

    struct GraphicsModule final {
        std::string name;
        std::optional<ShaderFunction> vertexFunction;
        std::optional<ShaderFunction> fragmentFunction;
    };

    struct ComputeModule final {
        std::string name;
        std::optional<ShaderFunction> computeFunction;
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

    std::optional<GraphicsModule> loadGraphicsModule(const ShaderModuleDescriptor& descriptor) noexcept;

    std::optional<ComputeModule> loadComputeModule(const ShaderModuleDescriptor& descriptor) noexcept;

  private:
    vax::Logger _logger = vax::Logger("ShaderLibrary");

    std::unique_ptr<const ReflectionProcessor> _reflectionProcessor = std::make_unique<ReflectionProcessor>();

    Slang::ComPtr<slang::IGlobalSession> _globalSession;

    Slang::ComPtr<slang::ISession> _session;

    std::optional<ShaderFunction>
    _loadEntryPoint(slang::IModule* module, const std::string& name, VkShaderStageFlags stageFlags) noexcept;
};

} // namespace vax::vk