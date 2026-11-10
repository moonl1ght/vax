#pragma once

#include "device.h"
#include "vulkanCommon.h"
#include <memory>
#include <utility>
#include <vector>
#include "texture.h"

namespace vax::vk {
class RenderDestination final {
  public:
    std::vector<VkFramebuffer> framebuffers;

    explicit RenderDestination(
        const Device& device,
        std::unique_ptr<Texture> depthTexture,
        std::vector<Texture> textures,
        std::vector<Texture> maskTextures,
        std::vector<VkFramebuffer> framebuffers
    )
        : _device(device)
        , _depthTexture(std::move(depthTexture))
        , _textures(std::move(textures))
        , _maskTextures(std::move(maskTextures))
        , framebuffers(std::move(framebuffers)) {}

    RenderDestination(const RenderDestination& other) = delete;
    RenderDestination& operator=(const RenderDestination& other) = delete;

    RenderDestination(RenderDestination&& other) = default;
    RenderDestination& operator=(RenderDestination&& other) = default;

    ~RenderDestination() { _destroy(); }

    const std::vector<Texture>& textures() const { return _textures; }

    const Texture& depthTexture() const { return *_depthTexture; }

    const std::vector<Texture>& maskTextures() const { return _maskTextures; }

  private:
    std::reference_wrapper<const Device> _device;
    std::vector<Texture> _textures;
    std::unique_ptr<Texture> _depthTexture;
    std::vector<Texture> _maskTextures;

    void _destroy();
};
} // namespace vax::vk