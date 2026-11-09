#pragma once

#include "descriptorSetLayout.h"
#include "device.h"
#include "logger.h"

namespace vax::vk {
class DescriptorSetLayoutBuilder final {
  public:
    explicit DescriptorSetLayoutBuilder(const vax::vk::Device& device, std::string name)
        : _device(device)
        , _name(name) {}

    ~DescriptorSetLayoutBuilder() {}

    DescriptorSetLayoutBuilder(const DescriptorSetLayoutBuilder&) = delete;
    DescriptorSetLayoutBuilder& operator=(const DescriptorSetLayoutBuilder&) = delete;
    DescriptorSetLayoutBuilder(DescriptorSetLayoutBuilder&& other) = delete;
    DescriptorSetLayoutBuilder& operator=(DescriptorSetLayoutBuilder&& other) = delete;

    void addBinding(const VkDescriptorSetLayoutBinding& binding) noexcept;

    std::optional<DescriptorSetLayout>
    build(VkDescriptorSetLayoutCreateFlags flags = 0);

  private:
    vax::Logger _logger = vax::Logger("DescriptorSetLayoutBuilder");
    std::reference_wrapper<const vax::vk::Device> _device;
    std::vector<VkDescriptorSetLayoutBinding> _bindings;
    std::string _name;
};
} // namespace vax::vk