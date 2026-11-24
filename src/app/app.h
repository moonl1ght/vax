#pragma once

#include "appMode.h"
#include "editor.h"
#include "frameTime.h"
#include "inputController.h"
#include "logger.h"
#include "renderEngine.h"
#include <memory>

namespace vax::app {

class App final {
  public:
    App() { _inputController = InputController(); };
    ~App() {};

    void setup() noexcept;

    void run() noexcept;

  private:
    dev::Logger _logger = dev::Logger("App");

    InputController _inputController;
    AppMode _appMode = AppMode::EventDriven;
    time::FrameTime _frameTime;

    std::unique_ptr<editor::Editor> _editor;
    std::unique_ptr<gfx::RenderEngine> _renderEngine;

    void _mainLoop();
    void _loopUpdate();
    void _cleanup();
    void _updateTimestamp();
};
} // namespace vax::app