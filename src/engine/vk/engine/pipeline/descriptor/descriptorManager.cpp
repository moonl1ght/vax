#include "descriptorManager.h"
#include "descriptorSetLayoutBuilder.h"

using namespace vax::vk;

bool DescriptorManager::setup() noexcept {
    VkDescriptorPoolCreateFlags flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    auto poolDescriptor = PoolDescriptor::bindlessPoolDescriptor(_poolLimits);
    VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = flags,
        .maxSets = poolDescriptor.maxSets,
        .poolSizeCount = static_cast<uint32_t>(poolDescriptor.poolSizes.size()),
        .pPoolSizes = poolDescriptor.poolSizes.data(),
    };
    VkDescriptorPool descriptorPool;
    if (!VK_CHECK(vkCreateDescriptorPool(_device.get().vkDevice(), &poolInfo, nullptr, &descriptorPool))) {
        _logger.error("Failed to create descriptor pool!");
        return false;
    }
    _descriptorPool = descriptorPool;

    auto layoutBuilder = DescriptorSetLayoutBuilder(_device.get(), "bindlessDescriptorSet");
    constexpr VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                                       VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                                                       VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    for (const auto& binding : PoolDescriptor::bindlessBindings(_poolLimits)) {
        layoutBuilder.addBinding(binding, bindlessFlags);
    }
    auto layout = layoutBuilder.build(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);
    if (!layout) {
        _logger.error("Failed to create descriptor set layout!");
        return false;
    }
    _descriptorSetLayout = layout.value();

    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &_descriptorSetLayout,
    };
    auto result = vkAllocateDescriptorSets(_device.get().vkDevice(), &allocInfo, &_descriptorSet);
    if (result != VK_SUCCESS) {
        _logger.error("Failed to allocate descriptor set!");
        return false;
    }
    _descriptorRegistry->use(_descriptorSet);
    return true;
}