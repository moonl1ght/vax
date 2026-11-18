#pragma once

#include "device.h"
#include <optional>
#include <string>

namespace vax::vk {
class Sampler final {
  public:
    static std::optional<vax::vk::Sampler>
    createSampler(const vax::vk::Device& device, std::string name, VkSamplerCreateInfo samplerInfo) noexcept;

    static std::optional<vax::vk::Sampler> linear(const vax::vk::Device& device) noexcept;

    static std::optional<vax::vk::Sampler> cube(const vax::vk::Device& device) noexcept;

    explicit Sampler(VkSampler vkSampler, const vax::vk::Device& device)
        : _vkSampler(vkSampler)
        , _device(device) {}

    Sampler(const Sampler& other) = delete;

    Sampler(Sampler&& other) noexcept
        : _device(other._device)
        , _vkSampler(other._vkSampler) {
        other._vkSampler = VK_NULL_HANDLE;
    }

    Sampler& operator=(const Sampler& other) = delete;

    Sampler& operator=(Sampler&& other) noexcept {
        if (this != &other) {
            if (_vkSampler != VK_NULL_HANDLE) {
                vkDestroySampler(_device.get().vkDevice(), _vkSampler, nullptr);
            }
            _device = other._device;
            _vkSampler = other._vkSampler;
            other._vkSampler = VK_NULL_HANDLE;
        }
        return *this;
    }

    ~Sampler() { vkDestroySampler(_device.get().vkDevice(), _vkSampler, nullptr); }

    VkSampler vkSampler() const { return _vkSampler; }

  private:
    std::reference_wrapper<const vax::vk::Device> _device;

    VkSampler _vkSampler = VK_NULL_HANDLE;
};
} // namespace vax::vk