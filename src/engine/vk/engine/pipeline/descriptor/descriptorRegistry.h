#pragma once

#include "indexAllocator.h"
#include "poolDescriptor.h"
#include <span>
#include <vulkan/vulkan.h>

namespace vax::vk {

class Texture;
class Device;

class DescriptorRegistry {
  public:
    struct RegistrationBatch final {
        std::span<const Texture* const> storageTextures;
        std::span<const Texture* const> sampledTextures;
        bool contiguous = false;
    };

    struct RegistrationResult final {
        std::optional<std::vector<uint32_t>> storageTextureIndices;
        std::optional<std::vector<uint32_t>> sampledTextureIndices;
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

    std::optional<std::vector<uint32_t>>
    registerStorageTexture(const std::vector<const Texture*>& textures, bool contiguous = false) noexcept;

    std::optional<std::vector<uint32_t>>
    registerSampledTexture(const std::vector<const Texture*>& textures, bool contiguous = false) noexcept;

    bool releaseStorageTexture(uint32_t index) noexcept { return _storageTextures.release(index); }

    bool releaseSampledTexture(uint32_t index) noexcept { return _sampledTextures.release(index); }

  private:
    std::reference_wrapper<const vax::vk::Device> _device;

    IndexAllocator _storageTextures;
    IndexAllocator _sampledTextures;

    VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;

    std::vector<VkWriteDescriptorSet> _writes;

    std::optional<std::vector<uint32_t>> _registerTexture(
        std::span<const Texture* const> textures,
        std::vector<VkDescriptorImageInfo>& imageInfos,
        VkDescriptorType descriptorType,
        IndexAllocator& indexAllocator,
        uint32_t binding,
        bool update,
        bool contiguous
    ) noexcept;
};

} // namespace vax::vk
