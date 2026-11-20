#pragma once

#include "device.h"
#include "texture.h"
#include "textureManager.h"
#include "vulkanCommon.h"
#include <memory>
#include <utility>
#include <vector>

namespace vax::rhi {
class RenderDestination final {
  public:
    std::vector<VkFramebuffer> framebuffers;

    explicit RenderDestination(
        const Device& device,
        TextureManager::TextureResource* depthTexture,
        std::vector<TextureManager::TextureResource*> colorTextures,
        std::vector<TextureManager::TextureResource*> maskTextures,
        std::vector<VkFramebuffer> framebuffers
    ) noexcept
        : _device(device)
        , _depthTexture(depthTexture)
        , _colorTextures(std::move(colorTextures))
        , _maskTextures(std::move(maskTextures))
        , framebuffers(std::move(framebuffers)) {}

    RenderDestination(const RenderDestination& other) = delete;
    RenderDestination& operator=(const RenderDestination& other) = delete;
    RenderDestination(RenderDestination&& other) noexcept = default;
    RenderDestination& operator=(RenderDestination&& other) noexcept = default;

    ~RenderDestination() { _destroy(); }

    const std::vector<TextureManager::TextureResource*>& colorTextures() const noexcept { return _colorTextures; }

    const TextureManager::TextureResource* depthTexture() const noexcept { return _depthTexture; }

    const std::vector<TextureManager::TextureResource*>& maskTextures() const noexcept { return _maskTextures; }

  private:
    std::reference_wrapper<const Device> _device;

    std::vector<TextureManager::TextureResource*> _colorTextures;
    TextureManager::TextureResource* _depthTexture;
    std::vector<TextureManager::TextureResource*> _maskTextures;

    void _destroy() noexcept;
};
} // namespace vax::rhi