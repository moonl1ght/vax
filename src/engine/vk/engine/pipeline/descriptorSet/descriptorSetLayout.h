#pragma once

#include "device.h"

namespace vax::vk {
struct DescriptorSetLayout final {
  public:
    explicit DescriptorSetLayout(
        const vax::vk::Device& device, VkDescriptorSetLayout vkDescriptorSetLayout, const std::string& name
    ) noexcept
        : _device(device)
        , _vkDescriptorSetLayout(vkDescriptorSetLayout)
        , _name(name) {}

    ~DescriptorSetLayout() { vkDestroyDescriptorSetLayout(_device.get().vkDevice(), _vkDescriptorSetLayout, nullptr); };

    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout(DescriptorSetLayout&&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

    bool operator==(const DescriptorSetLayout& other) const noexcept {
        return _vkDescriptorSetLayout == other._vkDescriptorSetLayout && _name == other._name;
    }

    VkDescriptorSetLayout getVkDescriptorSetLayout() const { return _vkDescriptorSetLayout; }

    const std::string& getName() const { return _name; }

  private:
    std::reference_wrapper<const vax::vk::Device> _device;
    std::string _name;
    VkDescriptorSetLayout _vkDescriptorSetLayout = VK_NULL_HANDLE;
};
} // namespace vax::vk