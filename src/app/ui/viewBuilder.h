#pragma once

#include "inputController.h"
#include "renderer.h"
#include "roverView.h"
#include "uiEngine.h"
#include "vkEngine.h"
#include "windowController.h"

namespace vax::ui {

class MenuView;
class PhysicsDemoMenuView;

class ViewBuilder {
  public:
    ViewBuilder(
        UIEngine& uiEngine,
        WindowController& windowController,
        InputController& inputController,
        engine::Renderer& renderer,
        vk::Engine& engine
    )
        : _uiEngine(uiEngine)
        , _windowController(windowController)
        , _inputController(inputController)
        , _renderer(renderer)
        , _engine(engine) {}

    ~ViewBuilder() = default;

    std::unique_ptr<MenuView> buildMenuView();

    std::unique_ptr<RoverView> buildRoverView();

    std::unique_ptr<PhysicsDemoMenuView> buildPhysicsDemoMenuView();

  private:
    std::reference_wrapper<UIEngine> _uiEngine;
    std::reference_wrapper<WindowController> _windowController;
    std::reference_wrapper<InputController> _inputController;
    std::reference_wrapper<engine::Renderer> _renderer;
    std::reference_wrapper<vk::Engine> _engine;
};

} // namespace vax::ui