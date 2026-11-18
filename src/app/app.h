#pragma once

#include "appMode.h"
#include "frameTime.h"
#include "inputController.h"
#include "logger.h"
#include "renderingEngine.h"
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

    rendering::RenderingEngine* getRenderingEngine() const { return _renderingEngine.get(); }

  private:
    Logger _logger = vax::Logger("App");

    InputController _inputController;
    AppMode _appMode = vax::AppMode::EventDriven;
    engine::FrameTime _frameTime;

    std::unique_ptr<WindowController> _windowController;
    std::unique_ptr<ui::ViewBuilder> _viewBuilder;
    std::unique_ptr<ui::ViewManager> _viewManager;
    std::unique_ptr<rendering::RenderingEngine> _renderingEngine;

    void _mainLoop();
    void _loopUpdate();
    void _cleanup();
    void _updateTimestamp();
};
} // namespace vax