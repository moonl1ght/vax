#pragma once

#include "editor.h"
#include "logger.h"
#include "renderer.h"
#include "resourceManager.h"
#include "vkEngine.h"

namespace vax::gfx {

class RenderEngine final {
  public:
    explicit RenderEngine() noexcept {};

    ~RenderEngine() = default;

    RenderEngine(const RenderEngine& other) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;
    RenderEngine(RenderEngine&& other) noexcept = delete;
    RenderEngine& operator=(RenderEngine&& other) noexcept = delete;

    bool setup(editor::Editor* editor) noexcept;

    void waitIdle() noexcept;

    void cleanup() noexcept;

    rhi::VulkanEngine& vkEngine() noexcept { return *_vkEngine; }

    rhi::ResourceManager& resourceManager() noexcept { return *_resourceManager; }

    gfx::Renderer& renderer() noexcept { return *_renderer; }

  private:
    dev::Logger _logger = dev::Logger("RenderEngine");

    std::unique_ptr<rhi::VulkanEngine> _vkEngine;
    std::unique_ptr<gfx::Renderer> _renderer;
    std::unique_ptr<rhi::ResourceManager> _resourceManager;
};

} // namespace vax::gfx