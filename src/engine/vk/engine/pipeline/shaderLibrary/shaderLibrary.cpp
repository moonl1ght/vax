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

void ShaderLibrary::loadSession(const std::string& shaderDirectory, Profile profile) noexcept {
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

void ShaderLibrary::loadModule(const ModuleDescriptor& descriptor) noexcept {
    auto module = _session->loadModule(descriptor.name.c_str());
    if (module == nullptr) {
        _logger.error("Failed to load module: ", descriptor.name);
        return;
    }

    for (const EntryPointDescriptor& entryPoint : descriptor.entryPoints) {
        auto result = _loadEntryPoint(module, entryPoint.name, entryPoint.stageFlags);
        if (result == std::nullopt) {
            _logger.error("Failed to load entry point: ", entryPoint.name);
            return;
        }
    }
}

std::optional<ShaderLibrary::CompilationResult> ShaderLibrary::_loadEntryPoint(
    slang::IModule* module, const std::string& name, VkShaderStageFlags stageFlags
) noexcept {
    CompilationResult compilationResult;

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
    compilationResult.spirv.assign(spirvData, spirvData + spirvSize);

    slang::ShaderReflection* reflection = linkedProgram->getLayout();

    Slang::ComPtr<slang::IMetadata> metadata;
    result = linkedProgram->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        _logger.error("Failed to get entry point metadata: ", name);
        return std::nullopt;
    }

    ReflectionProcessor::ReflectionContext context{reflection, _globalSession, stageFlags, metadata, _logger};

    compilationResult.reflectionData = _reflectionProcessor->process(context);

    return compilationResult;
}