#pragma once

#include "buffer.h"
#include "commandBuffer.h"
#include "texture.h"
#include "textureManager.h"

namespace vax::vk {
class TextureLoader final {
  public:
    explicit TextureLoader(const vax::vk::Device& device, TextureManager& textureManager) noexcept
        : _device(device)
        , _textureManager(textureManager) {};

    TextureLoader(const TextureLoader& other) = delete;
    TextureLoader& operator=(const TextureLoader& other) = delete;
    TextureLoader(TextureLoader&& other) noexcept = delete;
    TextureLoader& operator=(TextureLoader&& other) noexcept = delete;

    std::optional<TextureManager::TextureResource> loadTexture(std::string path) const noexcept;

    // for PNG and JPG textures
    std::optional<TextureManager::TextureResource>
    loadTexture(std::string name, std::span<unsigned char> data) const noexcept;

    std::optional<TextureManager::TextureResource>
    loadRawRGBATexture(std::string name, std::span<const unsigned char> pixels, int width, int height) const noexcept;

  private:
    vax::Logger _logger = vax::Logger("TextureLoader");

    std::reference_wrapper<const vax::vk::Device> _device;
    std::reference_wrapper<TextureManager> _textureManager;

    std::optional<TextureManager::TextureResource>
    _loadTexture(std::string name, const unsigned char* pixels, int texWidth, int texHeight) const noexcept;

    std::optional<TextureManager::TextureResource> _loadKTXTexture(std::string path) const noexcept;
};
} // namespace vax::vk