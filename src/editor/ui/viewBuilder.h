#pragma once

#include "editorView.h"
#include "gridWorldView.h"
#include "inputController.h"
#include "mainView.h"
#include "physicsDemoView.h"
#include "uiEngine.h"
#include "windowController.h"

namespace vax::gfx {
class RenderEngine;
class EditorRenderer_old;
class EditorRenderer;
} // namespace vax::gfx

namespace vax::editor {
class PhysicsDemoMenuView;

class ViewBuilder {
  public:
    ViewBuilder(
        WindowController& windowController,
        app::InputController& inputController,
        gfx::EditorRenderer_old& editorRenderer_old,
        gfx::EditorRenderer& editorRenderer,
        gfx::RenderEngine& renderEngine
    )
        : _windowController(windowController)
        , _inputController(inputController)
        , _editorRenderer_old(editorRenderer_old)
        , _editorRenderer(editorRenderer)
        , _renderEngine(renderEngine) {}

    ~ViewBuilder() = default;

    std::unique_ptr<MainView> buildMainView();

    std::unique_ptr<GridWorldView> buildGridWorldView();

    std::unique_ptr<PhysicsDemoMenuView> buildPhysicsDemoMenuView();

    std::unique_ptr<PhysicsDemoView> buildPhysicsDemoView();

    std::unique_ptr<editor::EditorView> buildPhysicsSphereDemoView();

  private:
    std::reference_wrapper<WindowController> _windowController;
    std::reference_wrapper<app::InputController> _inputController;
    std::reference_wrapper<gfx::EditorRenderer_old> _editorRenderer_old;
    std::reference_wrapper<gfx::EditorRenderer> _editorRenderer;
    std::reference_wrapper<gfx::RenderEngine> _renderEngine;
};

} // namespace vax::editor