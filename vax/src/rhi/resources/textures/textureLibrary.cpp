#include "textureLibrary.h"

using namespace vax::rhi;

std::optional<TextureLibrary::TextureHandle> TextureLibrary::addTexture(std::unique_ptr<Texture> texture) noexcept {
    auto [iter, success] = _textures.try_emplace(texture->id(), std::move(texture));
    if (!success) {
        _logger.error("Failed to add into library texture with id: ", texture->id());
        return std::nullopt;
    }
    return TextureHandle{
        .id = iter->first,
        .texture = iter->second.get(),
    };
}

void TextureLibrary::removeTexture(const std::string& id) noexcept { _textures.erase(id); }