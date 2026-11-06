#pragma once

#include "logger.h"
#include <slang.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace vax::vk {

struct ShaderReflectionData final {
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings;
    std::vector<VkPushConstantRange> pushConstants;
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