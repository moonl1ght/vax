#include "reflectionProcessor.h"
#include <algorithm>
#include <slang-com-ptr.h>

using namespace vax::vk;

constexpr VkDescriptorType mapSlangTypeToVulkan(slang::TypeReflection* type) {
    switch (type->getKind()) {
    case slang::TypeReflection::Kind::ConstantBuffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case slang::TypeReflection::Kind::SamplerState:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    case slang::TypeReflection::Kind::Resource: {
        const auto shape = type->getResourceShape();
        const bool isReadOnly = type->getResourceAccess() == SLANG_RESOURCE_ACCESS_READ;

        switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK) {
        case SLANG_STRUCTURED_BUFFER:
        case SLANG_BYTE_ADDRESS_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case SLANG_TEXTURE_BUFFER:
            return isReadOnly ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case SLANG_ACCELERATION_STRUCTURE:
            return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        case SLANG_TEXTURE_SUBPASS:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        default:
            break;
        }

        if (shape & SLANG_TEXTURE_COMBINED_FLAG) {
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        return isReadOnly ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

struct DescriptorBindingInfo final {
    uint32_t count = 1;
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
};

DescriptorBindingInfo resolveBindingInfo(slang::TypeReflection* type, vax::Logger& logger) {
    DescriptorBindingInfo info{};

    if (type->getKind() == slang::TypeReflection::Kind::Array) {
        size_t arraySize = type->getElementCount();

        if (arraySize == 0) [[unlikely]] {
            logger.warning("Trully bindless array better to avoid");
            constexpr uint32_t maxBindlessCount = 1024;
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

struct BindingLocation final {
    uint32_t set = 0;
    uint32_t binding = 0;
};

bool hasNoUabAttribute(slang::VariableLayoutReflection* var, ReflectionProcessor::ReflectionContext& context) {
    slang::VariableReflection* variable = var->getVariable();
    return variable && variable->findUserAttributeByName(context.globalSession, "NoUAB");
}

BindingLocation accumulateLocation(slang::VariableLayoutReflection* var, BindingLocation base) {
    constexpr auto slot = slang::ParameterCategory::DescriptorTableSlot;
    return {
        .set = base.set + static_cast<uint32_t>(var->getBindingSpace(slot)) +
               static_cast<uint32_t>(var->getOffset(slang::ParameterCategory::SubElementRegisterSpace)),
        .binding = base.binding + static_cast<uint32_t>(var->getOffset(slot)),
    };
}

void addBinding(
    ReflectionProcessor::ReflectionContext& context,
    ShaderReflectionData& data,
    BindingLocation location,
    VkDescriptorType type,
    uint32_t count,
    bool noUab
) {
    auto& sets = data.sets;
    if (location.set >= sets.size()) {
        sets.resize(location.set + 1);
    }
    if (noUab) {
        sets[location.set].isUpdateAfterBind = false;
    }

    auto& setBindings = sets[location.set].bindings;
    for (VkDescriptorSetLayoutBinding& existing : setBindings) {
        if (existing.binding != location.binding) {
            continue;
        }
        if (existing.descriptorType != type) {
            context.logger.warning("Different descriptor types at set ", location.set, " binding ", location.binding);
            return;
        }
        existing.descriptorCount = std::max(existing.descriptorCount, count);
        return;
    }

    VkDescriptorSetLayoutBinding binding{};
    binding.binding = location.binding;
    binding.descriptorType = type;
    binding.descriptorCount = count;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    setBindings.push_back(binding);
}

void addPushConstantRange(ShaderReflectionData& data, uint32_t offset, uint32_t size, VkShaderStageFlags stageFlags) {
    if (!data.pushConstants) {
        data.pushConstants = VkPushConstantRange{stageFlags, offset, size};
        return;
    }

    VkPushConstantRange& range = *data.pushConstants;
    const uint32_t end = std::max(range.offset + range.size, offset + size);
    range.offset = std::min(range.offset, offset);
    range.size = end - range.offset;
}

VkDescriptorType
resolveConstantBufferType(slang::VariableLayoutReflection* var, ReflectionProcessor::ReflectionContext& context) {
    slang::VariableReflection* variable = var->getVariable();
    const bool isDynamic = variable && variable->findUserAttributeByName(context.globalSession, "DynamicOffset");
    return isDynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

void collectParameter(
    slang::VariableLayoutReflection* var,
    BindingLocation base,
    ReflectionProcessor::ReflectionContext& context,
    ShaderReflectionData& data,
    bool noUab = false
) {
    const BindingLocation location = accumulateLocation(var, base);
    noUab = noUab || hasNoUabAttribute(var, context);
    slang::TypeLayoutReflection* typeLayout = var->getTypeLayout();

    switch (typeLayout->getKind()) {
    case slang::TypeReflection::Kind::Struct:
        for (unsigned int i = 0; i < typeLayout->getFieldCount(); ++i) {
            collectParameter(typeLayout->getFieldByIndex(i), location, context, data, noUab);
        }
        return;
    case slang::TypeReflection::Kind::ConstantBuffer:
    case slang::TypeReflection::Kind::ParameterBlock: {
        if (var->getCategory() == slang::ParameterCategory::PushConstantBuffer) {
            const size_t size = typeLayout->getElementTypeLayout()->getSize();
            addPushConstantRange(data, 0, static_cast<uint32_t>(size), context.stageFlags);
            return;
        }

        slang::VariableLayoutReflection* container = typeLayout->getContainerVarLayout();
        if (container->getTypeLayout()->getSize(slang::ParameterCategory::DescriptorTableSlot) > 0) {
            addBinding(
                context,
                data,
                accumulateLocation(container, location),
                resolveConstantBufferType(var, context),
                1,
                noUab
            );
        }
        collectParameter(typeLayout->getElementVarLayout(), location, context, data, noUab);

        if (typeLayout->getKind() == slang::TypeReflection::Kind::ParameterBlock) {
            const uint32_t blockSet = accumulateLocation(container, location).set;
            const char* typeName = typeLayout->getElementTypeLayout()->getName();
            if (blockSet < data.sets.size() && data.sets[blockSet].name.empty() && typeName) {
                data.sets[blockSet].name = typeName;
            }
        }
        return;
    }
    default:
        break;
    }

    if (typeLayout->getSize(slang::ParameterCategory::DescriptorTableSlot) == 0) {
        return;
    }

    const DescriptorBindingInfo bindingInfo = resolveBindingInfo(var->getType(), context.logger);
    if (bindingInfo.type == VK_DESCRIPTOR_TYPE_MAX_ENUM) {
        const char* name = var->getName();
        context.logger.warning("Unsupported descriptor type of shader parameter: ", name ? name : "<unnamed>");
        return;
    }
    addBinding(context, data, location, bindingInfo.type, bindingInfo.count, noUab);
}

ShaderReflectionData ReflectionProcessor::process(ReflectionContext& context) const noexcept {
    ShaderReflectionData data;

    collectParameter(context.reflection->getGlobalParamsVarLayout(), {}, context, data);

    slang::EntryPointReflection* entryPointReflection = context.reflection->getEntryPointByIndex(0);
    const BindingLocation entryPointBase = accumulateLocation(entryPointReflection->getVarLayout(), {});

    for (unsigned int i = 0; i < entryPointReflection->getParameterCount(); ++i) {
        slang::VariableLayoutReflection* param = entryPointReflection->getParameterByIndex(i);

        constexpr auto uniform = slang::ParameterCategory::Uniform;
        const size_t uniformSize = param->getTypeLayout()->getSize(uniform);
        if (uniformSize > 0) {
            addPushConstantRange(
                data,
                static_cast<uint32_t>(param->getOffset(uniform)),
                static_cast<uint32_t>(uniformSize),
                context.stageFlags
            );
        }

        collectParameter(param, entryPointBase, context, data);
    }

    for (ShaderReflectionData::SetReflectionData& set : data.sets) {
        std::sort(set.bindings.begin(), set.bindings.end(), [](const auto& a, const auto& b) {
            return a.binding < b.binding;
        });
    }
    return data;
}