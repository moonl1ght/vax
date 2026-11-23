#pragma once

#include "inputController.h"
#include "logger.h"
#include "uiEngine.h"
#include "viewBuilder.h"
#include "viewManager.h"
#include "windowController.h"
#include <memory>

namespace vax::gfx {
class RenderEngine;
} // namespace vax::gfx

namespace vax::editor {

class Editor {
  public:
    Editor(app::InputController& inputController) noexcept
        : _inputController(inputController) {};

    ~Editor() noexcept = default;

    Editor(const Editor& other) = delete;
    Editor& operator=(const Editor& other) = delete;
    Editor(Editor&& other) noexcept = delete;
    Editor& operator=(Editor&& other) noexcept = delete;

    bool setup() noexcept;

    void cleanup() noexcept;

    void loadMainView(gfx::RenderEngine& renderEngine) noexcept;

    void createUIEngine(rhi::VulkanEngine& vkEngine) noexcept;

    UIEngine& uiEngine() noexcept { return *_uiEngine; }

    const UIEngine& uiEngine() const noexcept { return *_uiEngine; }

    WindowController& windowController() noexcept { return *_windowController; }

    const WindowController& windowController() const noexcept { return *_windowController; }

    ViewManager& viewManager() noexcept { return *_viewManager; }

    const ViewManager& viewManager() const noexcept { return *_viewManager; }

  private:
    dev::Logger _logger = dev::Logger("Editor");

    std::reference_wrapper<app::InputController> _inputController;

    std::unique_ptr<WindowController> _windowController;
    std::unique_ptr<ViewBuilder> _viewBuilder;
    std::unique_ptr<ViewManager> _viewManager;
    std::unique_ptr<UIEngine> _uiEngine;
};

} // namespace vax::editor