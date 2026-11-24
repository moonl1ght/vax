#include "viewBuilder.h"
#include "editorRenderer.h"
#include "mainView.h"
#include "physicsDemoMenu.h"

using namespace vax;
using namespace vax::editor;

std::unique_ptr<MainView> ViewBuilder::buildMainView() { return std::make_unique<MainView>(_editorRenderer_old.get()); }

std::unique_ptr<GridWorldView> ViewBuilder::buildGridWorldView() {
    return std::make_unique<GridWorldView>(
        _windowController.get(), _editorRenderer_old.get(), _renderEngine.get(), _inputController.get()
    );
}

std::unique_ptr<PhysicsDemoMenuView> ViewBuilder::buildPhysicsDemoMenuView() {
    return std::make_unique<PhysicsDemoMenuView>(_editorRenderer_old.get());
}

std::unique_ptr<PhysicsDemoView> ViewBuilder::buildPhysicsDemoView() {
    return std::make_unique<PhysicsDemoView>(
        _editorRenderer_old.get(), _renderEngine.get(), _windowController.get(), _inputController.get()
    );
}

std::unique_ptr<editor::EditorView> ViewBuilder::buildPhysicsSphereDemoView() {
    return std::make_unique<editor::EditorView>(_editorRenderer_old.get());
}