#pragma once

#include "appMode.h"
#include "editor.h"
#include "frameTime.h"
#include "inputController.h"
#include "logger.h"
#include "renderEngine.h"
#include "simulationEngine.h"
#include <memory>

namespace vax::app {

class App final {
  public:
    App() { _inputController = InputController(); };
    ~App() {};

    void setup() noexcept;

    void run() noexcept;

    gfx::RenderEngine* getRenderEngine() const { return _renderEngine.get(); }

  private:
    dev::Logger _logger = dev::Logger("App");

    InputController _inputController;
    AppMode _appMode = AppMode::EventDriven;
    time::FrameTime _frameTime;

    std::unique_ptr<editor::Editor> _editor;
    std::unique_ptr<gfx::RenderEngine> _renderEngine;
    std::unique_ptr<sim::SimulationEngine> _simulationEngine;

    void _mainLoop();
    void _loopUpdate();
    void _cleanup();
    void _updateTimestamp();
};
} // namespace vax::app