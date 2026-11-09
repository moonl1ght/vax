#pragma once

#include "device.h"
#include <utility>

namespace vax::vk {
struct ShaderModule {
    VkShaderModule vkModule;

    explicit ShaderModule(const vax::vk::Device& device, VkShaderModule vkModule) noexcept
        : _device(device)
        , vkModule(vkModule) {}

    ~ShaderModule() {
        if (vkModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(_device.get().vkDevice, vkModule, nullptr);
        }
    }

    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;

    ShaderModule(ShaderModule&& other) noexcept
        : vkModule(std::exchange(other.vkModule, VK_NULL_HANDLE))
        , _device(other._device) {}

    ShaderModule& operator=(ShaderModule&& other) noexcept {
        if (this != &other) {
            if (vkModule != VK_NULL_HANDLE) {
                vkDestroyShaderModule(_device.get().vkDevice, vkModule, nullptr);
            }
            vkModule = std::exchange(other.vkModule, VK_NULL_HANDLE);
            _device = other._device;
        }
        return *this;
    }

  private:
    std::reference_wrapper<const vax::vk::Device> _device;
};

class ShaderModuleBuilder final {
  public:
    ShaderModuleBuilder(const vax::vk::Device& device)
        : _device(device) {}

    std::optional<VkShaderModule> build(const std::string& filename);

    std::optional<ShaderModule> build(const std::vector<uint32_t>& spirv);

  private:
    vax::Logger _logger = vax::Logger("ShaderModuleBuilder");

    std::reference_wrapper<const vax::vk::Device> _device;

    std::vector<char> _readFile(const std::string& filename) const;
};
} // namespace vax::vk