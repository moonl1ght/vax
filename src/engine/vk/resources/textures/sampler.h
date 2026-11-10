#pragma once

#include "device.h"
#include "vulkanCommon.h"
#include "logger.h"
#include <optional>
#include <string>

namespace vax::vk {
class Sampler final {
  public:
    VkSampler vkSampler = VK_NULL_HANDLE;

    static std::optional<vax::vk::Sampler>
    createSampler(const vax::vk::Device& device, std::string name, VkSamplerCreateInfo samplerInfo);

    explicit Sampler(VkSampler vkSampler, const vax::vk::Device& device)
        : vkSampler(vkSampler)
        , _device(device) {}

    Sampler(const Sampler& other) = delete;

    Sampler(Sampler&& other) noexcept
        : _device(other._device)
        , vkSampler(other.vkSampler) {
        other.vkSampler = VK_NULL_HANDLE;
    }

    Sampler& operator=(const Sampler& other) = delete;

    Sampler& operator=(Sampler&& other) noexcept {
        if (this != &other) {
            if (vkSampler != VK_NULL_HANDLE) {
                vkDestroySampler(_device.get().vkDevice, vkSampler, nullptr);
            }
            _device = other._device;
            vkSampler = other.vkSampler;
            other.vkSampler = VK_NULL_HANDLE;
        }
        return *this;
    }

    ~Sampler() { vkDestroySampler(_device.get().vkDevice, vkSampler, nullptr); }

    void setName(const std::string& name) { _name = name; }

  private:
    vax::Logger _logger = vax::Logger("Sampler");
    std::string _name;
    std::reference_wrapper<const vax::vk::Device> _device;
};
} // namespace vax::vk