#pragma once

#include "device.h"
#include "vulkanCommon.h"
#include "logger.h"
#include <cstdint>
#include <optional>
#include <string>
#include "texture.h"
#include "textureManager.h"

namespace vax::vk {
class CommandBuffer;
class CommandManager;
} // namespace vax::vk

namespace vax::vk {
class TextureFactory final {
  public:
    struct TextureCreateInfo {
        std::string name;
        VkFormat format;
        math::SizeUI size;
        VkImageUsageFlags imageUsageFlags;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        VkImageCreateFlags flags = 0;
        uint32_t numLayers = 1;
        uint32_t numMips = 1;
    };

    explicit TextureFactory(const vax::vk::Device& device, TextureManager* textureManager)
        : _device(device)
        , _textureManager(textureManager) {};

    explicit TextureFactory(const vax::vk::Device& device)
        : _device(device)
        , _textureManager(nullptr) {};

    TextureFactory(const TextureFactory& other) = delete;
    TextureFactory& operator=(const TextureFactory& other) = delete;
    TextureFactory(TextureFactory&& other) noexcept = delete;
    TextureFactory& operator=(TextureFactory&& other) noexcept = delete;

    std::optional<vax::vk::TextureManager::TextureResource> makeDepthTexture(VkFormat format, math::SizeUI size);

    std::optional<Texture> makeDepthTextureDetached(VkFormat format, math::SizeUI size);

    std::optional<vax::vk::TextureManager::TextureResource> makeTexture(const TextureCreateInfo& createInfo);

    std::optional<Texture> makeTextureDetached(const TextureCreateInfo& createInfo);

  private:
    vax::Logger _logger = vax::Logger("TextureFactory");
    std::reference_wrapper<const Device> _device;
    TextureManager* _textureManager;
};
} // namespace vax::vk