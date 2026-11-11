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

slang::IModule* ShaderLibrary::_loadModule(const std::string& name) noexcept {
    Slang::ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = _session->loadModule(name.c_str(), diagnostics.writeRef());
    if (module == nullptr) {
        _logger.error(
            "Failed to load module: ",
            name,
            "\n",
            diagnostics ? static_cast<const char*>(diagnostics->getBufferPointer()) : ""
        );
    }
    return module;
}

std::optional<ShaderLibrary::PipelineModule>
ShaderLibrary::loadPipelineModule(const ShaderModuleDescriptor& descriptor) noexcept {
    if (descriptor.entryPoints.size() != 1) {
        _logger.error("Compute module must have exactly one entry point: ", descriptor.name);
        return std::nullopt;
    }
    slang::IModule* module = _loadModule(descriptor.name);
    if (module == nullptr) {
        return std::nullopt;
    }
    std::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints(descriptor.entryPoints.size());
    std::vector<slang::IComponentType*> components{module};
    std::vector<VkShaderStageFlags> stages;
    components.reserve(descriptor.entryPoints.size() + 1);
    stages.reserve(descriptor.entryPoints.size());
    for (size_t i = 0; i < descriptor.entryPoints.size(); ++i) {
        const EntryPointDescriptor& entryPoint = descriptor.entryPoints[i];
        if (SLANG_FAILED(module->findEntryPointByName(entryPoint.name.c_str(), entryPoints[i].writeRef()))) {
            _logger.error("Failed to find entry point: ", entryPoint.name, " in ", descriptor.name);
            return std::nullopt;
        }
        components.push_back(entryPoints[i]);
        stages.push_back(entryPoint.stageFlags);
    }

    auto diagnosticText = [](slang::IBlob* blob) {
        return blob ? static_cast<const char*>(blob->getBufferPointer()) : "unknown error";
    };

    Slang::ComPtr<slang::IComponentType> program;
    Slang::ComPtr<slang::IBlob> diagnostics;
    if (SLANG_FAILED(_session->createCompositeComponentType(
            components.data(), static_cast<SlangInt>(components.size()), program.writeRef(), diagnostics.writeRef()
        ))) {
        _logger.error(
            "Failed to create composite component type: ", descriptor.name, "\n", diagnosticText(diagnostics)
        );
        return std::nullopt;
    }

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    if (SLANG_FAILED(program->link(linkedProgram.writeRef(), diagnostics.writeRef()))) {
        _logger.error("Failed to link program: ", descriptor.name, "\n", diagnosticText(diagnostics));
        return std::nullopt;
    }

    PipelineModule pipelineModule;
    pipelineModule.name = descriptor.name;
    pipelineModule.functions.reserve(descriptor.entryPoints.size());
    for (size_t i = 0; i < descriptor.entryPoints.size(); ++i) {
        Slang::ComPtr<slang::IBlob> spirvBlob;
        if (SLANG_FAILED(linkedProgram->getEntryPointCode(
                static_cast<SlangInt>(i), 0, spirvBlob.writeRef(), diagnostics.writeRef()
            ))) {
            _logger.error(
                "Failed to get entry point code: ", descriptor.entryPoints[i].name, "\n", diagnosticText(diagnostics)
            );
            return std::nullopt;
        }
        const auto* spirvData = static_cast<const uint32_t*>(spirvBlob->getBufferPointer());
        const size_t spirvSize = spirvBlob->getBufferSize() / sizeof(uint32_t);
        pipelineModule.functions.push_back(
            ShaderFunction{
            .name = descriptor.entryPoints[i].name,
            .stageFlags = descriptor.entryPoints[i].stageFlags,
            .spirv = std::vector<uint32_t>(spirvData, spirvData + spirvSize),
            }
        );
    }

    ReflectionProcessor::ReflectionContext context{
        .reflection = linkedProgram->getLayout(),
        .globalSession = _globalSession,
        .entryPointStages = stages,
        .logger = _logger,
    };
    pipelineModule.reflectionData = _reflectionProcessor->process(context);
    return pipelineModule;
}