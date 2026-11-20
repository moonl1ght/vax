#pragma once

#include "appMode.h"
#include "frameTime.h"
#include "inputController.h"
#include "logger.h"
#include "renderEngine.h"
#include "viewBuilder.h"
#include "viewManager.h"
#include "vkEngine.h"
#include "windowController.h"
#include <memory>

namespace vax {

class App final {
  public:
    App() { _inputController = InputController(); };
    ~App() {};

    void setup() noexcept;

    void run() noexcept;

    gfx::RenderEngine* getRenderEngine() const { return _renderEngine.get(); }

  private:
    Logger _logger = vax::Logger("App");

    InputController _inputController;
    AppMode _appMode = vax::AppMode::EventDriven;
    gfx::FrameTime _frameTime;

    std::unique_ptr<WindowController> _windowController;
    std::unique_ptr<ui::ViewBuilder> _viewBuilder;
    std::unique_ptr<ui::ViewManager> _viewManager;
    std::unique_ptr<gfx::RenderEngine> _renderEngine;

    void _mainLoop();
    void _loopUpdate();
    void _cleanup();
    void _updateTimestamp();
};
} // namespace vax