#pragma once

#include "logger.h"
#include "textureLoader.h"
#include <string>

namespace vax::rhi {

class TextureGenerator final {
  public:
    explicit TextureGenerator(rhi::TextureLoader& textureLoader)
        : _textureLoader(textureLoader) {};

    ~TextureGenerator() = default;

    TextureGenerator(const TextureGenerator& other) = delete;
    TextureGenerator(TextureGenerator&& other) noexcept = delete;
    TextureGenerator& operator=(const TextureGenerator& other) = delete;
    TextureGenerator& operator=(TextureGenerator&& other) noexcept = delete;

    std::unique_ptr<Texture>
    generateCheckerboardTexture(const std::string& name, int width, int height, int cellSize) const noexcept;

  private:
    dev::Logger _logger = dev::Logger("TextureGenerator");
    std::reference_wrapper<TextureLoader> _textureLoader;
};
} // namespace vax::rhi