#pragma once

#include "inputController.h"
#include "logger.h"
#include "luna.h"
#include "renderer.h"
#include "uiEngine.h"
#include "vkEngine.h"
#include "frameTime.h"
#include "windowController.h"
#include "appMode.h"
#include "viewBuilder.h"
#include "viewManager.h"

namespace vax {

class App final {
  public:
    App() { _inputController = vax::InputController(); };
    ~App() {};

    bool run();

    vax::vk::Engine* getEngine() const { return _engine.get(); }

  private:
    vax::Logger _logger = vax::Logger("App");

    vax::InputController _inputController;
    vax::AppMode _appMode = vax::AppMode::EventDriven;
    vax::engine::FrameTime _frameTime;

    std::unique_ptr<vax::WindowController> _windowController;
    std::unique_ptr<vax::vk::Engine> _engine;
    std::unique_ptr<vax::engine::Renderer> _renderer;
    std::unique_ptr<vax::ui::UIEngine> _uiEngine;
    std::unique_ptr<vax::ui::ViewBuilder> _viewBuilder;
    std::unique_ptr<vax::ui::ViewManager> _viewManager;

    bool _setup();
    void _mainLoop();
    void _cleanup();
    void _loopByEventUpdate();
    void _loopContinuousUpdate();
    void _updateTimestamp();
    void _testCuda() const;
    void _testPhysics() const;
};
} // namespace vax