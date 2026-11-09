#pragma once

#include "device.h"

namespace vax::vk {
class DescriptorSetLayout final {
  public:
    explicit DescriptorSetLayout(const vax::vk::Device& device, VkDescriptorSetLayout vkDescriptorSetLayout)
        : _device(device)
        , _vkDescriptorSetLayout(vkDescriptorSetLayout) {}

    ~DescriptorSetLayout() { vkDestroyDescriptorSetLayout(_device.get().vkDevice, _vkDescriptorSetLayout, nullptr); }

    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

    DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
        : _device(other._device)
        , _vkDescriptorSetLayout(other._vkDescriptorSetLayout) {
        other._vkDescriptorSetLayout = VK_NULL_HANDLE;
    }

    DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept {
        if (this != &other) {
            _device = other._device;
            _vkDescriptorSetLayout = other._vkDescriptorSetLayout;
            other._vkDescriptorSetLayout = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkDescriptorSetLayout getVkDescriptorSetLayout() const { return _vkDescriptorSetLayout; }

    void addEntryPointName(const std::string& entryPointName) { _entryPointNames.push_back(entryPointName); }

    const std::vector<std::string>& getEntryPointNames() const { return _entryPointNames; }

  private:
    std::reference_wrapper<const vax::vk::Device> _device;
    std::vector<std::string> _entryPointNames;
    VkDescriptorSetLayout _vkDescriptorSetLayout = VK_NULL_HANDLE;
};
} // namespace vax::vk