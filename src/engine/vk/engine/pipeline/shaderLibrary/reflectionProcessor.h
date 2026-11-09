#pragma once

#include "logger.h"
#include "poolDescriptor.h"
#include <slang.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace vax::vk {

struct ShaderReflectionData final {
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings;
    std::vector<bool> noUabSets;
    std::vector<VkPushConstantRange> pushConstants;

    bool isUpdateAfterBindSet(size_t set) const noexcept {
        return set < bindings.size() && !(set < noUabSets.size() && noUabSets[set]);
    }

    std::vector<VkDescriptorBindingFlags> bindingFlags(size_t set) const {
        std::vector<VkDescriptorBindingFlags> flags;
        if (set >= bindings.size()) {
            return flags;
        }
        const bool uab = isUpdateAfterBindSet(set);
        flags.reserve(bindings[set].size());
        for (const VkDescriptorSetLayoutBinding& binding : bindings[set]) {
            VkDescriptorBindingFlags f = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
            if (uab && supportsUpdateAfterBind(binding.descriptorType)) {
                f |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
            }
            flags.push_back(f);
        }
        return flags;
    }
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