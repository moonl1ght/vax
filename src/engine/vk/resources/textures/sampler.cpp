#include "sampler.h"

using namespace vax::vk;

std::optional<Sampler>
Sampler::createSampler(const vk::Device& device, std::string name, VkSamplerCreateInfo samplerInfo) {
    VkSampler vkSampler;
    if (vkCreateSampler(device.vkDevice(), &samplerInfo, nullptr, &vkSampler) != VK_SUCCESS) {
        Logger::getInstance().error("failed to create texture sampler!");
        return std::nullopt;
    }
    if (!name.empty()) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_SAMPLER,
            .objectHandle = reinterpret_cast<size_t>(vkSampler),
            .pObjectName = name.c_str(),
        };
        vax::vk::pfnSetDebugUtilsObjectNameEXT(device.vkDevice(), &nameInfo);
    }
    return Sampler(vkSampler, device);
}