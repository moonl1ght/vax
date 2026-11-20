#pragma once

#include "gridWorldView.h"
#include "inputController.h"
#include "physicsDemoView.h"
#include "renderEngine.h"
#include "uiEngine.h"
#include "windowController.h"

namespace vax::ui {

class StartingView;
class PhysicsDemoMenuView;

class ViewBuilder {
  public:
    ViewBuilder(
        WindowController& windowController,
        InputController& inputController,
        gfx::RenderEngine& renderEngine
    )
        : _windowController(windowController)
        , _inputController(inputController)
        , _renderEngine(renderEngine) {}

    ~ViewBuilder() = default;

    std::unique_ptr<StartingView> buildStartingView();

    std::unique_ptr<GridWorldView> buildGridWorldView();

    std::unique_ptr<PhysicsDemoMenuView> buildPhysicsDemoMenuView();

    std::unique_ptr<PhysicsDemoView> buildPhysicsDemoView();

  private:
    std::reference_wrapper<WindowController> _windowController;
    std::reference_wrapper<InputController> _inputController;
    std::reference_wrapper<gfx::RenderEngine> _renderEngine;
};

} // namespace vax::ui