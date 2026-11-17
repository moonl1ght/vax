#pragma once

#include "descriptorRegistry.h"
#include "device.h"
#include "logger.h"
#include "poolDescriptor.h"
#include "sampler.h"
#include "shaderResourceHandle.h"
#include "texture.h"
#include <memory>
#include <vector>

namespace vax::vk {
class TextureFactory;
} // namespace vax::vk

namespace vax::vk {
class TextureManager final {
  public:
    enum class GlobalSampler : uint32_t { PBRSampler = 0, CubeMapSampler = 1 };

    enum class TextureRegistrationType : uint32_t {
        SampledImage = 0,
        StorageImage = 1,
    };

    struct TextureResource final {
        TextureId id;
        TextureRegistrationType registrationType;
        Texture texture;
        SampledImageShaderHandle sampledShaderHandle = {};
        StorageImageShaderHandle storageShaderHandle = {};

        ShaderResourceIndex getShaderResourceIndex() const {
            switch (registrationType) {
            case TextureRegistrationType::SampledImage:
                return sampledShaderHandle.index;
            case TextureRegistrationType::StorageImage:
                return storageShaderHandle.index;
            }
        }
    };

    struct SamplerResource final {
        Sampler sampler;
        SamplerShaderHandle shaderHandle;
    };

    struct TextureCreateInfo final {
        std::string name;
        VkFormat format;
        math::SizeUI size;
        VkImageUsageFlags imageUsageFlags;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        VkImageCreateFlags flags = 0;
        uint32_t numLayers = 1;
        uint32_t numMips = 1;
    };

    explicit TextureManager(const vk::Device& device, DescriptorRegistry& descriptorRegistry) noexcept
        : _device(device)
        , _descriptorRegistry(descriptorRegistry) {
        _globalSamplers.reserve(MAX_SAMPLERS);
    };

    ~TextureManager() { cleanup(); }

    TextureManager(const TextureManager& other) = delete;
    TextureManager(TextureManager&& other) noexcept = delete;
    TextureManager& operator=(const TextureManager& other) = delete;
    TextureManager& operator=(TextureManager&& other) noexcept = delete;

    void cleanup() noexcept;

    bool setup() noexcept;

    TextureResource* findTexture(TextureId id) noexcept;

    const SamplerResource* getGlobalSampler(GlobalSampler sampler) const noexcept;

    void loadAllTextures(vax::vk::CommandBuffer& commandBuffer) noexcept;

    void resetAllTexturesStagingBuffers() noexcept;

    TextureResource* createDepthTexture(VkFormat format, vax::math::SizeUI size) noexcept;

    TextureResource* createTexture(const TextureCreateInfo& createInfo) noexcept;

  private:
    vax::Logger _logger = vax::Logger("TextureManager");

    std::reference_wrapper<const vk::Device> _device;
    std::reference_wrapper<DescriptorRegistry> _descriptorRegistry;

    std::vector<std::unique_ptr<TextureResource>> _textures;
    std::vector<SamplerResource> _globalSamplers;

    bool _registerLastTexture() noexcept;
};
} // namespace vax::vk