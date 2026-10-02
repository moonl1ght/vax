#pragma once

#include "buffer.h"
#include "commandBuffer.h"
#include "texture.h"
#include "textureManager.h"

namespace vax::vk {
class TextureLoader final {
  public:
    TextureLoader(const vax::vk::Device& device, TextureManager& textureManager)
        : _device(device)
        , _textureManager(textureManager) {};

    TextureLoader(const TextureLoader& other) = delete;
    TextureLoader& operator=(const TextureLoader& other) = delete;
    TextureLoader(TextureLoader&& other) noexcept = delete;
    TextureLoader& operator=(TextureLoader&& other) noexcept = delete;

    std::optional<TextureManager::TextureResource> loadTexture(std::string path);

    std::optional<TextureManager::TextureResource>
    loadTexture(std::string name, std::span<unsigned char> data);

  private:
    vax::Logger _logger = vax::Logger("TextureLoader");

    std::reference_wrapper<const vax::vk::Device> _device;
    std::reference_wrapper<TextureManager> _textureManager;

    std::optional<TextureManager::TextureResource> _loadTexture(
        std::string name,
        unsigned char* pixels,
        int texWidth,
        int texHeight,
        int texChannels
    );

    std::optional<TextureManager::TextureResource> _loadKTXTexture(std::string path);
};
} // namespace vax::vk