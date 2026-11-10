#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace vax::vk {

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
    static PoolDescriptor defaultPoolDescriptor() {
        return {
            {
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 500},
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