#pragma once

#include "logger.h"
#include "renderer.h"
#include "resourceManager.h"
#include "uiEngine.h"
#include "vkEngine.h"
#include "windowController.h"
#include <memory>

namespace vax::rendering {
class RenderingEngine final {
  public:
    explicit RenderingEngine() noexcept {};

    ~RenderingEngine() = default;

    RenderingEngine(const RenderingEngine& other) = delete;
    RenderingEngine& operator=(const RenderingEngine& other) = delete;
    RenderingEngine(RenderingEngine&& other) noexcept = delete;
    RenderingEngine& operator=(RenderingEngine&& other) noexcept = delete;

    bool setup(vax::WindowController& windowController) noexcept;

    void waitIdle() noexcept;

    void cleanup() noexcept;

    vax::vk::Engine& vkEngine() noexcept { return *_vkEngine; }

    vax::vk::ResourceManager& resourceManager() noexcept { return *_resourceManager; }

    vax::engine::Renderer& renderer() noexcept { return *_renderer; }

    vax::ui::UIEngine& uiEngine() noexcept { return *_uiEngine; }

  private:
    vax::Logger _logger = vax::Logger("RenderingEngine");

    std::unique_ptr<vax::vk::Engine> _vkEngine;
    std::unique_ptr<vax::engine::Renderer> _renderer;
    std::unique_ptr<vax::ui::UIEngine> _uiEngine;
    std::unique_ptr<vax::vk::ResourceManager> _resourceManager;
};
} // namespace vax::rendering