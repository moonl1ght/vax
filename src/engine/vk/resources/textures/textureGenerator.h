#pragma once

#include "logger.h"
#include "textureLoader.h"
#include <string>

namespace vax::vk {

class TextureGenerator final {
  public:
    explicit TextureGenerator(vk::TextureLoader& textureLoader)
        : _textureLoader(textureLoader) {};

    ~TextureGenerator() = default;

    TextureGenerator(const TextureGenerator& other) = delete;
    TextureGenerator(TextureGenerator&& other) noexcept = delete;
    TextureGenerator& operator=(const TextureGenerator& other) = delete;
    TextureGenerator& operator=(TextureGenerator&& other) noexcept = delete;

    std::optional<vk::TextureManager::TextureResource>
    generateCheckerboardTexture(const std::string& name, int width, int height, int cellSize) const noexcept;

  private:
    Logger _logger = Logger("TextureGenerator");
    std::reference_wrapper<TextureLoader> _textureLoader;
};
} // namespace vax::vk