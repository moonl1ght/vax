#pragma once

#include <algorithm>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace vax::vk {

struct PoolLimits final {
    uint32_t maxSampledImages = 500;
    uint32_t maxStorageImages = 500;

    PoolLimits(VkPhysicalDeviceProperties properties) {
        maxSampledImages = std::min(properties.limits.maxDescriptorSetSampledImages, static_cast<uint32_t>(500));
        maxStorageImages = std::min(properties.limits.maxDescriptorSetStorageImages, static_cast<uint32_t>(500));
    }
};

constexpr bool supportsUpdateAfterBind(VkDescriptorType type) noexcept {
    switch (type) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        return false;
    default:
        return true;
    }
}

enum class PoolType {
    DEFAULT,
    UAB,
};

struct PoolDescriptor final {
    static PoolDescriptor bindlessPoolDescriptor(const PoolLimits& limits) {
        return {
            {
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, limits.maxSampledImages},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, limits.maxStorageImages},
            },
            1
        };
    };

    static std::vector<VkDescriptorSetLayoutBinding> bindlessBindings(const PoolLimits& limits) {
        return {
            {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, limits.maxSampledImages, VK_SHADER_STAGE_ALL},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, limits.maxStorageImages, VK_SHADER_STAGE_ALL},
        };
    }

    static PoolDescriptor defaultPoolDescriptor() {
        return {
            {
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 500},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 500},
            },
            500
        };
    };

    std::vector<VkDescriptorPoolSize> poolSizes;
    uint32_t maxSets;
    PoolType type;

    bool poolSupportsUpdateAfterBind() const noexcept {
        for (const auto& poolSize : poolSizes) {
            if (!supportsUpdateAfterBind(poolSize.type)) {
                return false;
            }
        }
        return true;
    }
};

} // namespace vax::vk