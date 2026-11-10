#include "shaderLibrary.h"

using namespace vax::vk;

constexpr const char* profileToString(ShaderLibrary::Profile profile) {
    switch (profile) {
    case ShaderLibrary::Profile::sm_5_0:
        return "sm_5_0";
    case ShaderLibrary::Profile::sm_6_5:
        return "sm_6_5";
    case ShaderLibrary::Profile::sm_6_6:
        return "sm_6_6";
    }
    return "sm_5_0";
}

void ShaderLibrary::startSession(const std::string& shaderDirectory, Profile profile) noexcept {
    const char* searchPaths[] = {shaderDirectory.c_str()};

    slang::SessionDesc sessionDesc = {};
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = _globalSession->findProfile(profileToString(profile));
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    _globalSession->createSession(sessionDesc, _session.writeRef());
}

void ShaderLibrary::endSession() noexcept { _session = nullptr; }

std::optional<ShaderLibrary::GraphicsModule>
ShaderLibrary::loadGraphicsModule(const ShaderModuleDescriptor& descriptor) noexcept {
    Slang::ComPtr<slang::IBlob> diagnostics;
    auto module = _session->loadModule(descriptor.name.c_str(), diagnostics.writeRef());
    if (module == nullptr) {
        _logger.error(
            "Failed to load module: ",
            descriptor.name,
            "\n",
            diagnostics ? static_cast<const char*>(diagnostics->getBufferPointer()) : ""
        );
        return std::nullopt;
    }

    GraphicsModule graphicsModule{descriptor.name};
    for (const EntryPointDescriptor& entryPoint : descriptor.entryPoints) {
        auto result = _loadEntryPoint(module, entryPoint.name, entryPoint.stageFlags);
        if (result == std::nullopt) {
            _logger.error("Failed to load entry point: ", entryPoint.name);
            return std::nullopt;
        }
        switch (entryPoint.stageFlags) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            graphicsModule.vertexFunction = result.value();
            break;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            graphicsModule.fragmentFunction = result.value();
            break;
        default:
            _logger.error("Unsupported stage flags: ", entryPoint.stageFlags);
            return std::nullopt;
        }
    }

    return std::make_optional(graphicsModule);
}

std::optional<ShaderLibrary::ComputeModule>
ShaderLibrary::loadComputeModule(const ShaderModuleDescriptor& descriptor) noexcept {
    Slang::ComPtr<slang::IBlob> diagnostics;
    auto module = _session->loadModule(descriptor.name.c_str(), diagnostics.writeRef());
    if (module == nullptr) {
        _logger.error(
            "Failed to load module: ",
            descriptor.name,
            "\n",
            diagnostics ? static_cast<const char*>(diagnostics->getBufferPointer()) : ""
        );
        return std::nullopt;
    }

    ComputeModule computeModule{descriptor.name};
    if (descriptor.entryPoints.size() != 1) {
        _logger.error("Compute module must have exactly one entry point: ", descriptor.name);
        return std::nullopt;
    }
    auto result = _loadEntryPoint(module, descriptor.entryPoints[0].name, descriptor.entryPoints[0].stageFlags);
    if (result == std::nullopt) {
        _logger.error("Failed to load entry point: ", descriptor.entryPoints[0].name);
        return std::nullopt;
    }
    computeModule.computeFunction = result.value();

    return std::make_optional(computeModule);
}

std::optional<ShaderLibrary::ShaderFunction> ShaderLibrary::_loadEntryPoint(
    slang::IModule* module, const std::string& name, VkShaderStageFlags stageFlags
) noexcept {
    ShaderFunction shaderFunction;

    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    SlangResult result;
    result = module->findEntryPointByName(name.c_str(), entryPoint.writeRef());
    if (SLANG_FAILED(result)) {
        _logger.error("Failed to find entry point: ", name);
        return std::nullopt;
    }

    slang::IComponentType* components[] = {module, entryPoint};
    Slang::ComPtr<slang::IComponentType> program;
    Slang::ComPtr<slang::IBlob> diagnostics;

    result = _session->createCompositeComponentType(components, 2, program.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        if (diagnostics) {
            _logger.error("Failed to create composite component type: ", (const char*)diagnostics->getBufferPointer());
        } else {
            _logger.error("Unknown linking error");
        }
        return std::nullopt;
    }

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    result = program->link(linkedProgram.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        if (diagnostics) {
            _logger.error("Failed to link program: ", (const char*)diagnostics->getBufferPointer());
        } else {
            _logger.error("Unknown linking error");
        }
        return std::nullopt;
    }

    Slang::ComPtr<slang::IBlob> spirvBlob;
    result = linkedProgram->getEntryPointCode(0, 0, spirvBlob.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        if (diagnostics) {
            _logger.error("Failed to get entry point code: ", (const char*)diagnostics->getBufferPointer());
        } else {
            _logger.error("Unknown error");
        }
        return std::nullopt;
    }

    const uint32_t* spirvData = static_cast<const uint32_t*>(spirvBlob->getBufferPointer());
    size_t spirvSize = spirvBlob->getBufferSize() / sizeof(uint32_t);
    shaderFunction.spirv.assign(spirvData, spirvData + spirvSize);

    slang::ShaderReflection* reflection = linkedProgram->getLayout();

    Slang::ComPtr<slang::IMetadata> metadata;
    result = linkedProgram->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        _logger.error("Failed to get entry point metadata: ", name);
        return std::nullopt;
    }

    ReflectionProcessor::ReflectionContext context{reflection, _globalSession, stageFlags, metadata, _logger};

    shaderFunction.reflectionData = _reflectionProcessor->process(context);
    shaderFunction.name = name;
    shaderFunction.stageFlags = stageFlags;

    return shaderFunction;
}