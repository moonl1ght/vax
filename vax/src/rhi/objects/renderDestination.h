#pragma once

#include "device.h"
#include "texture.h"
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
        std::unique_ptr<Texture> depthTexture,
        std::vector<std::unique_ptr<Texture>> colorTextures,
        std::vector<std::unique_ptr<Texture>> maskTextures,
        std::vector<VkFramebuffer> framebuffers
    ) noexcept
        : _device(device)
        , _depthTexture(std::move(depthTexture))
        , _colorTextures(std::move(colorTextures))
        , _maskTextures(std::move(maskTextures))
        , framebuffers(std::move(framebuffers)) {}

    RenderDestination(const RenderDestination& other) = delete;
    RenderDestination& operator=(const RenderDestination& other) = delete;
    RenderDestination(RenderDestination&& other) noexcept = default;
    RenderDestination& operator=(RenderDestination&& other) noexcept = default;

    ~RenderDestination() { _destroy(); }

    const std::vector<std::unique_ptr<Texture>>& colorTextures() const noexcept { return _colorTextures; }

    const std::unique_ptr<Texture>& depthTexture() const noexcept { return _depthTexture; }

    const std::vector<std::unique_ptr<Texture>>& maskTextures() const noexcept { return _maskTextures; }

  private:
    std::reference_wrapper<const Device> _device;

    std::vector<std::unique_ptr<Texture>> _colorTextures;
    std::unique_ptr<Texture> _depthTexture;
    std::vector<std::unique_ptr<Texture>> _maskTextures;

    void _destroy() noexcept;
};
} // namespace vax::rhi