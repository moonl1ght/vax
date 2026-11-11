#pragma once

#include "indexAllocator.h"
#include "poolDescriptor.h"
#include <vulkan/vulkan.h>

namespace vax::vk {

class Texture;
class Device;

class DescriptorRegistry {
  public:
    struct RegistrationBatch final {
        const std::vector<const Texture*> storageTextures;
        const std::vector<const Texture*> sampledTextures;
    };

    struct RegistrationResult final {
        std::optional<uint32_t> storageTextureIndex;
        std::optional<uint32_t> sampledTextureIndex;
    };

    explicit DescriptorRegistry(const vax::vk::Device& device, const PoolLimits& poolLimits) noexcept
        : _device(device)
        , _storageTextures(poolLimits.maxStorageImages)
        , _sampledTextures(poolLimits.maxSampledImages) {};

    ~DescriptorRegistry() = default;

    DescriptorRegistry(const DescriptorRegistry&) = delete;
    DescriptorRegistry(DescriptorRegistry&&) noexcept = delete;
    DescriptorRegistry& operator=(const DescriptorRegistry&) = delete;
    DescriptorRegistry& operator=(DescriptorRegistry&&) noexcept = delete;

    void use(VkDescriptorSet descriptorSet) noexcept { _descriptorSet = descriptorSet; }

    RegistrationResult registerBatch(const RegistrationBatch& batch) noexcept;

    std::optional<uint32_t> registerStorageTexture(const std::vector<const Texture*>& textures) noexcept;

    std::optional<uint32_t> registerSampledTexture(const std::vector<const Texture*>& textures) noexcept;

  private:
    std::reference_wrapper<const vax::vk::Device> _device;

    IndexAllocator _storageTextures;
    IndexAllocator _sampledTextures;

    VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;

    std::vector<VkWriteDescriptorSet> _writes;

    std::optional<uint32_t> _registerTexture(
        const std::vector<const Texture*>& textures,
        std::vector<VkDescriptorImageInfo>& imageInfos,
        VkDescriptorType descriptorType,
        uint32_t binding,
        bool update
    ) noexcept;
};

} // namespace vax::vk