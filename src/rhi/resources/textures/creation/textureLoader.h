#pragma once

#include "buffer.h"
#include "commandBuffer.h"
#include "texture.h"
#include "textureFactory.h"

namespace vax::rhi {
class TextureLoader final {
  public:
    explicit TextureLoader(const vax::rhi::Device& device, DescriptorRegistry& descriptorRegistry) noexcept
        : _device(device)
        , _textureFactory(std::make_unique<TextureFactory>(device, descriptorRegistry)) {};

    TextureLoader(const TextureLoader& other) = delete;
    TextureLoader& operator=(const TextureLoader& other) = delete;
    TextureLoader(TextureLoader&& other) noexcept = delete;
    TextureLoader& operator=(TextureLoader&& other) noexcept = delete;

    std::unique_ptr<Texture> loadTexture(std::string path) const noexcept;

    // for PNG and JPG textures
    std::unique_ptr<Texture> loadTexture(std::string name, std::span<unsigned char> data) const noexcept;

    std::unique_ptr<Texture>
    loadRawRGBATexture(std::string name, std::span<const unsigned char> pixels, int width, int height) const noexcept;

  private:
    dev::Logger _logger = dev::Logger("TextureLoader");

    std::reference_wrapper<const vax::rhi::Device> _device;

    std::unique_ptr<TextureFactory> _textureFactory;

    std::unique_ptr<Texture>
    _loadTexture(std::string name, const unsigned char* pixels, int texWidth, int texHeight) const noexcept;

    std::unique_ptr<Texture> _loadKTXTexture(std::string path) const noexcept;
};
} // namespace vax::rhi