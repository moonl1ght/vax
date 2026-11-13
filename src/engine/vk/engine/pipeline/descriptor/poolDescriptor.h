#pragma once

#include "shaderConstants.h"
#include "vkUtils.h"
#include <algorithm>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace vax::vk {

#define MAX_SAMPLERS 30
#define MAX_SAMPLED_IMAGES 1024
#define MAX_STORAGE_IMAGES 1024
#define MAX_UNIFORM_BUFFERS 128
#define MAX_STORAGE_BUFFERS 1024
#define MAX_UNIFORM_BUFFERS_DYNAMIC 128

struct PoolLimits final {
    uint32_t maxSampledImages = MAX_SAMPLED_IMAGES;
    uint32_t maxStorageImages = MAX_STORAGE_IMAGES;
    uint32_t maxSamplers = MAX_SAMPLERS;
    uint32_t maxUniformBuffers = MAX_UNIFORM_BUFFERS;
    uint32_t maxUniformBuffersDynamic = MAX_UNIFORM_BUFFERS_DYNAMIC;
    uint32_t maxStorageBuffers = MAX_STORAGE_BUFFERS;
    uint32_t maxPerStageBindlessResources = 0;

    PoolLimits(const VkPhysicalDeviceProperties& properties, const VkPhysicalDeviceVulkan12Properties& vk12) {
        maxSampledImages = std::min({
            vk12.maxDescriptorSetUpdateAfterBindSampledImages,
            vk12.maxPerStageDescriptorUpdateAfterBindSampledImages,
            static_cast<uint32_t>(MAX_SAMPLED_IMAGES),
        });
        maxStorageImages = std::min({
            vk12.maxDescriptorSetUpdateAfterBindStorageImages,
            vk12.maxPerStageDescriptorUpdateAfterBindStorageImages,
            static_cast<uint32_t>(MAX_STORAGE_IMAGES),
        });
        maxSamplers = std::min({
            vk12.maxDescriptorSetUpdateAfterBindSamplers,
            vk12.maxPerStageDescriptorUpdateAfterBindSamplers,
            static_cast<uint32_t>(MAX_SAMPLERS),
        });
        maxStorageBuffers = std::min({
            vk12.maxDescriptorSetUpdateAfterBindStorageBuffers,
            vk12.maxPerStageDescriptorUpdateAfterBindStorageBuffers,
            static_cast<uint32_t>(MAX_STORAGE_BUFFERS),
        });
        maxPerStageBindlessResources = vk12.maxPerStageUpdateAfterBindResources;

        maxUniformBuffers = std::min({
            properties.limits.maxDescriptorSetUniformBuffers,
            properties.limits.maxPerStageDescriptorUniformBuffers,
            static_cast<uint32_t>(MAX_UNIFORM_BUFFERS),
        });
        maxUniformBuffersDynamic = std::min({
            properties.limits.maxDescriptorSetUniformBuffersDynamic,
            properties.limits.maxDescriptorSetStorageBuffersDynamic,
            static_cast<uint32_t>(MAX_UNIFORM_BUFFERS_DYNAMIC),
        });
    }

    uint64_t bindlessResourcesPerStage() const noexcept {
        return uint64_t{maxSampledImages} + maxStorageImages + maxSamplers + maxStorageBuffers;
    }

    bool fitsPerStageLimit() const noexcept { return bindlessResourcesPerStage() <= maxPerStageBindlessResources; }
};

struct PoolDescriptor final {
    static PoolDescriptor bindlessPoolDescriptor(const PoolLimits& limits) {
        return {
            {
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, limits.maxSampledImages},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, limits.maxStorageImages},
            {VK_DESCRIPTOR_TYPE_SAMPLER, limits.maxSamplers},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, limits.maxStorageBuffers},
            },
            1
        };
    };

    static PoolDescriptor uboPoolDescriptor(const PoolLimits& limits) {
        return {
            {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, limits.maxUniformBuffers},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, limits.maxUniformBuffers},
            },
            MAX_FRAMES_IN_FLIGHT
        };
    };

    static std::vector<VkDescriptorSetLayoutBinding> bindlessBindings(const PoolLimits& limits) {
        return {
            {
            SAMPLED_IMAGE_BINDING_INDEX,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            limits.maxSampledImages,
            VK_SHADER_STAGE_ALL,
            },
            {
            STORAGE_IMAGE_BINDING_INDEX,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            limits.maxStorageImages,
            VK_SHADER_STAGE_ALL,
            },
            {
            SAMPLER_BINDING_INDEX,
            VK_DESCRIPTOR_TYPE_SAMPLER,
            limits.maxSamplers,
            VK_SHADER_STAGE_ALL,
            },
            {
            STORAGE_BUFFER_BINDING_INDEX,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            limits.maxStorageBuffers,
            VK_SHADER_STAGE_ALL,
            },
        };
    }

    static std::vector<VkDescriptorSetLayoutBinding> uboBindings() {
        return {
            {
            FRAME_UNIFORM_BUFFER_INDEX,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_ALL,
            },
            {
            LIGHT_UNIFORM_BUFFER_INDEX,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_ALL,
            },
        };
    }

    std::vector<VkDescriptorPoolSize> poolSizes;
    uint32_t maxSets;
};

} // namespace vax::vk