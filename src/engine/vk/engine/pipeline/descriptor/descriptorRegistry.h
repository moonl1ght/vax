#pragma once

#include "indexAllocator.h"
#include "poolDescriptor.h"
#include "shaderResourceHandle.h"
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
        std::optional<std::vector<StorageHandle>> storageTextures;
        std::optional<std::vector<SampledHandle>> sampledTextures;
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

    std::optional<std::vector<StorageHandle>>
    registerStorageTexture(const std::vector<const Texture*>& textures, bool contiguous = false) noexcept;

    std::optional<std::vector<SampledHandle>>
    registerSampledTexture(const std::vector<const Texture*>& textures, bool contiguous = false) noexcept;

    bool releaseStorageTexture(StorageHandle handle) noexcept { return _storageTextures.release(handle.index); }

    bool releaseSampledTexture(SampledHandle handle) noexcept { return _sampledTextures.release(handle.index); }

  private:
    template <typename Handle>
    static std::optional<std::vector<Handle>>
    _toHandles(std::optional<std::vector<ShaderResourceIndex>> indices) noexcept {
        if (!indices) {
            return std::nullopt;
        }
        std::vector<Handle> handles;
        handles.reserve(indices->size());
        for (ShaderResourceIndex index : *indices) {
            handles.push_back(Handle{.index = index});
        }
        return handles;
    }

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
