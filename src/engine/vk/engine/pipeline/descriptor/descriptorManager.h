#pragma once

#include <descriptorRegistry.h>
#include <device.h>
#include <logger.h>
#include <poolDescriptor.h>

namespace vax::vk {

class DescriptorManager final {
  public:
    explicit DescriptorManager(const vax::vk::Device& device) noexcept
        : _device(device)
        , _poolLimits(_device.get().physicalDeviceProperties()) {
        _descriptorRegistry = std::make_unique<DescriptorRegistry>(_device, _poolLimits);
    };

    ~DescriptorManager() {
        vkDestroyDescriptorSetLayout(_device.get().vkDevice(), _descriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(_device.get().vkDevice(), _descriptorPool, nullptr);
    }

    DescriptorManager(const DescriptorManager&) = delete;
    DescriptorManager(DescriptorManager&&) noexcept = delete;
    DescriptorManager& operator=(const DescriptorManager&) = delete;
    DescriptorManager& operator=(DescriptorManager&&) noexcept = delete;

    bool setup() noexcept;

    VkDescriptorSetLayout descriptorSetLayout() const noexcept { return _descriptorSetLayout; }

    VkDescriptorSet descriptorSet() const noexcept { return _descriptorSet; }

    DescriptorRegistry& descriptorRegistry() const noexcept { return *_descriptorRegistry; }

  private:
    vax::Logger _logger = vax::Logger("DescriptorManager");

    std::reference_wrapper<const vax::vk::Device> _device;

    std::unique_ptr<DescriptorRegistry> _descriptorRegistry;

    const PoolLimits _poolLimits;

    VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
    VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
};

} // namespace vax::vk