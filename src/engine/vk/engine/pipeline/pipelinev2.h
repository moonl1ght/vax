#pragma once

#include <descriptorSetHandler.h>
#include <descriptorSetWriter.h>
#include <hashUtils.h>
#include <vulkan/vulkan_core.h>

namespace vax::vk {

struct PipelineLayoutKey final {
    std::vector<std::string> descriptorSetNames;
    std::vector<VkDescriptorSetLayout> setLayouts;
    std::optional<VkPushConstantRange> pushConstants;

    bool operator==(const PipelineLayoutKey& other) const noexcept {
        if (descriptorSetNames != other.descriptorSetNames || setLayouts != other.setLayouts ||
            pushConstants.has_value() != other.pushConstants.has_value()) {
            return false;
        }
        if (!pushConstants) {
            return true;
        }
        return pushConstants->stageFlags == other.pushConstants->stageFlags &&
               pushConstants->offset == other.pushConstants->offset && pushConstants->size == other.pushConstants->size;
    }
};

struct PipelineV2 final {
    PipelineLayoutKey pipelineLayoutKey;
    VkPipeline vkPipeline = VK_NULL_HANDLE;
    std::vector<std::string> descriptorSetNames;
};

struct PipelineHandle final {
    const PipelineV2* pipeline;
    VkPipelineLayout pipelineLayout;
    std::unordered_map<std::string, std::vector<DescriptorSetWriter>> descriptorSetWriters;
    std::unordered_map<std::string, std::vector<DescriptorSetHandler>> descriptorSetHandlers;
};

} // namespace vax::vk

template <> struct std::hash<vax::vk::PipelineLayoutKey> {
    size_t operator()(const vax::vk::PipelineLayoutKey& key) const noexcept {
        size_t seed = key.descriptorSetNames.size();
        for (const std::string& name : key.descriptorSetNames) {
            vax::hash_combine(seed, name);
        }
        for (const VkDescriptorSetLayout& layout : key.setLayouts) {
            vax::hash_combine(seed, layout);
        }
        if (key.pushConstants) {
            vax::hash_combine(seed, static_cast<uint32_t>(key.pushConstants->stageFlags));
            vax::hash_combine(seed, key.pushConstants->offset);
            vax::hash_combine(seed, key.pushConstants->size);
        }
        return seed;
    }
};
