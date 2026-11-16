#pragma once

#include "appMode.h"
#include "frameTime.h"
#include "inputController.h"
#include "logger.h"
#include "renderer.h"
#include "uiEngine.h"
#include "viewBuilder.h"
#include "viewManager.h"
#include "vkEngine.h"
#include "vulkanCommon.h"
#include "windowController.h"
#include <memory>

namespace vax {

class App final {
  public:
    App() { _inputController = vax::InputController(); };
    ~App() {};

    void setup() noexcept;

    void run() noexcept;

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
    void _loopUpdate();
    void _updateTimestamp();
};
} // namespace vax