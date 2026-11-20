#pragma once

#include "logger.h"
#include "renderer.h"
#include "resourceManager.h"
#include "uiEngine.h"
#include "vkEngine.h"
#include "windowController.h"

namespace vax::gfx {

class RenderEngine final {
  public:
    explicit RenderEngine() noexcept {};

    ~RenderEngine() = default;

    RenderEngine(const RenderEngine& other) = delete;
    RenderEngine& operator=(const RenderEngine& other) = delete;
    RenderEngine(RenderEngine&& other) noexcept = delete;
    RenderEngine& operator=(RenderEngine&& other) noexcept = delete;

    bool setup(vax::WindowController& windowController) noexcept;

    void waitIdle() noexcept;

    void cleanup() noexcept;

    rhi::Engine& vkEngine() noexcept { return *_vkEngine; }

    rhi::ResourceManager& resourceManager() noexcept { return *_resourceManager; }

    gfx::Renderer& renderer() noexcept { return *_renderer; }

    ui::UIEngine& uiEngine() noexcept { return *_uiEngine; }

  private:
    dev::Logger _logger = dev::Logger("RenderEngine");

    std::unique_ptr<vax::rhi::Engine> _vkEngine;
    std::unique_ptr<vax::gfx::Renderer> _renderer;
    std::unique_ptr<vax::ui::UIEngine> _uiEngine;
    std::unique_ptr<vax::rhi::ResourceManager> _resourceManager;
};

} // namespace vax::gfx