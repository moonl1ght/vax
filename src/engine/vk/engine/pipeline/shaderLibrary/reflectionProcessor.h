#pragma once

#include "hashUtils.h"
#include "logger.h"
#include <optional>
#include <slang.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace vax::vk {

struct ShaderReflectionData final {
    struct SetReflectionData final {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bool isUpdateAfterBind = true;
        std::string name;

        bool operator==(const SetReflectionData& other) const noexcept {
            if (isUpdateAfterBind != other.isUpdateAfterBind || bindings.size() != other.bindings.size()) {
                return false;
            }
            for (size_t i = 0; i < bindings.size(); ++i) {
                const VkDescriptorSetLayoutBinding& a = bindings[i];
                const VkDescriptorSetLayoutBinding& b = other.bindings[i];
                if (a.binding != b.binding || a.descriptorType != b.descriptorType ||
                    a.descriptorCount != b.descriptorCount || a.stageFlags != b.stageFlags ||
                    a.pImmutableSamplers != b.pImmutableSamplers) {
                    return false;
                }
            }
            return true;
        }
    };

    std::vector<SetReflectionData> sets;
    std::optional<VkPushConstantRange> pushConstants;
};

class ReflectionProcessor final {
  public:
    struct ReflectionContext final {
        slang::ShaderReflection* reflection;
        slang::IGlobalSession* globalSession;
        VkShaderStageFlags stageFlags;
        slang::IMetadata* metadata;
        vax::Logger& logger;
    };

    ReflectionProcessor() = default;
    ~ReflectionProcessor() = default;

    ReflectionProcessor(const ReflectionProcessor&) = delete;
    ReflectionProcessor(ReflectionProcessor&&) = delete;
    ReflectionProcessor& operator=(const ReflectionProcessor&) = delete;
    ReflectionProcessor& operator=(ReflectionProcessor&&) = delete;

    ShaderReflectionData process(ReflectionContext& context) const noexcept;
};

} // namespace vax::vk

template <> struct std::hash<vax::vk::ShaderReflectionData::SetReflectionData> {
    size_t operator()(const vax::vk::ShaderReflectionData::SetReflectionData& set) const noexcept {
        size_t seed = 0;
        vax::hash_combine(seed, set.isUpdateAfterBind);
        for (const VkDescriptorSetLayoutBinding& binding : set.bindings) {
            vax::hash_combine(seed, binding.binding);
            vax::hash_combine(seed, static_cast<uint32_t>(binding.descriptorType));
            vax::hash_combine(seed, binding.descriptorCount);
            vax::hash_combine(seed, static_cast<uint32_t>(binding.stageFlags));
            vax::hash_combine(seed, binding.pImmutableSamplers);
        }
        return seed;
    }
};