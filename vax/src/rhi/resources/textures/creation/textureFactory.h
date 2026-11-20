#pragma once

#include "logger.h"
#include "texture.h"
#include <random>
#include <uuid_v4.h>

namespace vax::rhi {

class TextureFactory final {
  public:
    struct TextureCreateInfo final {
        Texture::RegistrationType registrationType = Texture::RegistrationType::SampledImage;
        std::string name;
        VkFormat format;
        math::SizeUI size;
        VkImageUsageFlags imageUsageFlags;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        VkImageCreateFlags flags = 0;
        uint32_t numLayers = 1;
        uint32_t numMips = 1;
    };

    explicit TextureFactory(const rhi::Device& device, rhi::DescriptorRegistry& descriptorRegistry) noexcept
        : _device(device)
        , _descriptorRegistry(descriptorRegistry) {};

    TextureFactory(const TextureFactory& other) = delete;
    TextureFactory& operator=(const TextureFactory& other) = delete;
    TextureFactory(TextureFactory&& other) noexcept = delete;
    TextureFactory& operator=(TextureFactory&& other) noexcept = delete;

    std::unique_ptr<Texture> createTexture(const TextureCreateInfo& createInfo) noexcept;

    std::unique_ptr<Texture> createDepthTexture(
        VkFormat format,
        vax::math::SizeUI size,
        Texture::RegistrationType registrationType = Texture::RegistrationType::SampledImage
    ) noexcept;

  private:
    dev::Logger _logger = dev::Logger("TextureFactory");

    std::reference_wrapper<const rhi::Device> _device;
    std::reference_wrapper<rhi::DescriptorRegistry> _descriptorRegistry;

    UUIDv4::UUIDGenerator<std::mt19937_64> _uuidGenerator;
};

} // namespace vax::rhi