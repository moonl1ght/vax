#pragma once

#include "logger.h"
#include "texture.h"

namespace vax::rhi {
class TextureGpuUploader final {
  public:
    explicit TextureGpuUploader() noexcept {};

    TextureGpuUploader(const TextureGpuUploader& other) = delete;
    TextureGpuUploader& operator=(const TextureGpuUploader& other) = delete;
    TextureGpuUploader(TextureGpuUploader&& other) noexcept = delete;
    TextureGpuUploader& operator=(TextureGpuUploader&& other) noexcept = delete;

    void addTexture(Texture* texture) noexcept { _textures.emplace_back(texture); }

    void uploadTextures(rhi::CommandBuffer& commandBuffer) noexcept {
        for (auto& texture : _textures) {
            if (!texture) [[unlikely]] {
                _logger.warning("Texture is null");
                continue;
            }
            texture->loadStagingBuffer(commandBuffer);
        }
    }

    void resetAllTexturesStagingBuffers() noexcept {
        for (auto& texture : _textures) {
            if (!texture) [[unlikely]] {
                _logger.warning("Texture is null");
                continue;
            }
            texture->resetStagingBuffer();
        }
    }

  private:
    dev::Logger _logger = dev::Logger("TextureGpuUploader");

    std::vector<Texture*> _textures;
};
} // namespace vax::rhi