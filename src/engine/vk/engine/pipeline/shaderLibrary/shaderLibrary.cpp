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

constexpr VkDescriptorType mapSlangTypeToVulkan(slang::TypeReflection* type) {
    auto kind = type->getKind();

    switch (kind) {
    case slang::TypeReflection::Kind::ConstantBuffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case slang::TypeReflection::Kind::SamplerState:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    case slang::TypeReflection::Kind::Resource: {
        auto shape = type->getResourceShape();
        auto access = type->getResourceAccess();
        auto baseShape = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;

        if (baseShape == SLANG_STRUCTURED_BUFFER || baseShape == SLANG_BYTE_ADDRESS_BUFFER) {
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        }

        if (access == SLANG_RESOURCE_ACCESS_READ_WRITE) {
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        } else {
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        }
    }
    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

struct DescriptorBindingInfo final {
    uint32_t count = 1;
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
};

DescriptorBindingInfo ResolveBindingInfo(slang::TypeReflection* type, vax::Logger& logger) {
    DescriptorBindingInfo info{};

    if (type->getKind() == slang::TypeReflection::Kind::Array) {
        size_t arraySize = type->getElementCount();

        if (arraySize == 0) [[unlikely]] {
            logger.warning("Trully bindless array better to avoid");
            constexpr uint32_t maxBindlessCount = 10000;
            info.count = maxBindlessCount;
        } else {
            info.count = static_cast<uint32_t>(arraySize);
        }

        type = type->getElementType();
    } else {
        info.count = 1;
    }

    info.type = mapSlangTypeToVulkan(type);

    return info;
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

void ShaderLibrary::loadModule(const std::string& name) noexcept {
    auto module = _session->loadModule(name.c_str());
    if (module == nullptr) {
        _logger.error("Failed to load module: {}", name);
        return;
    }
}

std::optional<ShaderLibrary::CompilationResult> ShaderLibrary::_loadEntryPoint(
    Slang::ComPtr<slang::IModule> module, const std::string& name, VkShaderStageFlags stageFlags
) noexcept {
    CompilationResult compilationResult;

    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    SlangResult result;
    result = module->findEntryPointByName(name.c_str(), entryPoint.writeRef());
    if (SLANG_FAILED(result)) {
        _logger.error("Failed to find entry point: {}", name);
        return std::nullopt;
    }

    slang::IComponentType* components[] = {module, entryPoint};
    Slang::ComPtr<slang::IComponentType> program;
    Slang::ComPtr<slang::IBlob> diagnostics;

    result = _session->createCompositeComponentType(components, 2, program.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        if (diagnostics) {
            _logger.error(
                "Failed to create composite component type: {}", (const char*)diagnostics->getBufferPointer()
            );
        } else {
            _logger.error("Unknown linking error");
        }
        return std::nullopt;
    }

    Slang::ComPtr<slang::IBlob> spirvBlob;
    result = program->getEntryPointCode(0, 0, spirvBlob.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
        if (diagnostics) {
            _logger.error("Failed to get entry point code: {}", (const char*)diagnostics->getBufferPointer());
        } else {
            _logger.error("Unknown error");
        }
        return std::nullopt;
    }

    const uint32_t* spirvData = static_cast<const uint32_t*>(spirvBlob->getBufferPointer());
    size_t spirvSize = spirvBlob->getBufferSize() / sizeof(uint32_t);
    compilationResult.spirv.assign(spirvData, spirvData + spirvSize);

    slang::ShaderReflection* reflection = program->getLayout();
    unsigned int paramCount = reflection->getParameterCount();

    for (unsigned int i = 0; i < paramCount; ++i) {
        slang::VariableLayoutReflection* param = reflection->getParameterByIndex(i);
        auto category = param->getCategory();

        switch (category) {
        case slang::ParameterCategory::PushConstantBuffer: {
            VkPushConstantRange range{};
            range.stageFlags = stageFlags;
            range.offset = static_cast<uint32_t>(param->getOffset());
            range.size = static_cast<uint32_t>(param->getTypeLayout()->getSize());
            compilationResult.reflectionData.pushConstants.push_back(range);
            break;
        }
        case slang::ParameterCategory::DescriptorTableSlot: {
            unsigned int set = param->getBindingSpace();
            if (set >= compilationResult.reflectionData.bindings.size()) {
                compilationResult.reflectionData.bindings.resize(set + 1);
            }
            DescriptorBindingInfo bindingInfo = ResolveBindingInfo(param->getType(), _logger);

            VkDescriptorSetLayoutBinding binding{};
            binding.binding = param->getBindingIndex();
            binding.descriptorType = bindingInfo.type;
            binding.descriptorCount = bindingInfo.count;
            binding.stageFlags = stageFlags;

            if (binding.descriptorType != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
                compilationResult.reflectionData.bindings[set].push_back(binding);
            }
            break;
        }
        default:
            break;
        }
    }

    return compilationResult;
}