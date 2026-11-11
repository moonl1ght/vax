#include "descriptorSetLayoutBuilder.h"
#include "vkUtils.h"

using namespace vax::vk;
using namespace vax;

void DescriptorSetLayoutBuilder::addBinding(const VkDescriptorSetLayoutBinding& binding) noexcept {
    _bindings.push_back(binding);
}

std::optional<VkDescriptorSetLayout>
DescriptorSetLayoutBuilder::build(VkDescriptorSetLayoutCreateFlags flags) noexcept {
    uint32_t bindingCount = static_cast<uint32_t>(_bindings.size());
    std::vector<VkDescriptorBindingFlags> bindingFlags(bindingCount, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = bindingCount,
        .pBindingFlags = bindingFlags.data(),
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &bindingFlagsInfo,
        .flags = flags,
        .bindingCount = bindingCount,
        .pBindings = _bindings.data(),
    };

    VkDescriptorSetLayout descriptorSetLayout;
    if (!VK_CHECK(vkCreateDescriptorSetLayout(_device.get().vkDevice(), &layoutInfo, nullptr, &descriptorSetLayout))) {
        _logger.error("Failed to create descriptor set layout!");
        return std::nullopt;
    }

    if (!_name.empty()) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
            .objectHandle = reinterpret_cast<size_t>(descriptorSetLayout),
            .pObjectName = _name.data(),
        };
        vax::vk::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice(), &nameInfo);
    }
    return std::make_optional<VkDescriptorSetLayout>(descriptorSetLayout);
}