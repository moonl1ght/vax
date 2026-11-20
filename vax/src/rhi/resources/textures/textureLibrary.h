#pragma once

#include "logger.h"
#include "texture.h"
#include <memory>

namespace vax::rhi {
class TextureFactory;
} // namespace vax::rhi

namespace vax::rhi {
class TextureLibrary final {
  public:
    struct TextureHandle final {
        std::string id;
        Texture* texture;
    };

    explicit TextureLibrary() noexcept {};

    ~TextureLibrary() = default;

    TextureLibrary(const TextureLibrary& other) = delete;
    TextureLibrary(TextureLibrary&& other) noexcept = delete;
    TextureLibrary& operator=(const TextureLibrary& other) = delete;
    TextureLibrary& operator=(TextureLibrary&& other) noexcept = delete;

    std::optional<TextureHandle> addTexture(std::unique_ptr<Texture> texture) noexcept;

    void removeTexture(const std::string& id) noexcept;

  private:
    dev::Logger _logger = dev::Logger("TextureLibrary");

    std::unordered_map<std::string, std::unique_ptr<Texture>> _textures;
};
} // namespace vax::rhi