#pragma once

#include "gridWorldView.h"
#include "inputController.h"
#include "physicsDemoView.h"
#include "uiEngine.h"
#include "windowController.h"
#include "startingView.h"

namespace vax::gfx {
class RenderEngine;
class Renderer;
} // namespace vax::gfx

namespace vax::editor {

class StartingView;
class PhysicsDemoMenuView;

class ViewBuilder {
  public:
    ViewBuilder(
        WindowController& windowController,
        app::InputController& inputController,
        gfx::Renderer& renderer,
        gfx::RenderEngine& renderEngine
    )
        : _windowController(windowController)
        , _inputController(inputController)
        , _renderer(renderer)
        , _renderEngine(renderEngine) {}

    ~ViewBuilder() = default;

    std::unique_ptr<StartingView> buildStartingView();

    std::unique_ptr<GridWorldView> buildGridWorldView();

    std::unique_ptr<PhysicsDemoMenuView> buildPhysicsDemoMenuView();

    std::unique_ptr<PhysicsDemoView> buildPhysicsDemoView();

  private:
    std::reference_wrapper<WindowController> _windowController;
    std::reference_wrapper<app::InputController> _inputController;
    std::reference_wrapper<gfx::Renderer> _renderer;
    std::reference_wrapper<gfx::RenderEngine> _renderEngine;
};

} // namespace vax::editor