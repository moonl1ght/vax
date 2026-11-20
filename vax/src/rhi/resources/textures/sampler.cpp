#include "sampler.h"

using namespace vax::rhi;

std::optional<Sampler>
Sampler::createSampler(const rhi::Device& device, std::string name, VkSamplerCreateInfo samplerInfo) noexcept {
    VkSampler vkSampler;
    if (vkCreateSampler(device.vkDevice(), &samplerInfo, nullptr, &vkSampler) != VK_SUCCESS) {
        return std::nullopt;
    }
    if (!name.empty()) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_SAMPLER,
            .objectHandle = reinterpret_cast<size_t>(vkSampler),
            .pObjectName = name.c_str(),
        };
        vax::rhi::pfnSetDebugUtilsObjectNameEXT(device.vkDevice(), &nameInfo);
    }
    return Sampler(vkSampler, device);
}

std::optional<Sampler> Sampler::linear(const rhi::Device& device) noexcept {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device.vkPhysicalDevice(), &properties);
    VkSamplerCreateInfo globalSamplerInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    return createSampler(device, "linear_sampler", globalSamplerInfo);
}

std::optional<Sampler> Sampler::cube(const rhi::Device& device) noexcept {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device.vkPhysicalDevice(), &properties);
    VkSamplerCreateInfo globalCubeMapSamplerInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    return createSampler(device, "cube_map_sampler", globalCubeMapSamplerInfo);
}