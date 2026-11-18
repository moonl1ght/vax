#include "viewBuilder.h"
#include "physicsDemoMenu.h"
#include "startingView.h"

using namespace vax::ui;

std::unique_ptr<StartingView> ViewBuilder::buildStartingView() {
    return std::make_unique<StartingView>(_renderer.get());
}

std::unique_ptr<GridWorldView> ViewBuilder::buildGridWorldView() {
    return std::make_unique<GridWorldView>(
        _uiEngine.get(),
        _windowController.get(),
        _renderer.get(),
        _engine.get(),
        _inputController.get(),
        _resourceManager.get()
    );
}

std::unique_ptr<PhysicsDemoMenuView> ViewBuilder::buildPhysicsDemoMenuView() {
    return std::make_unique<PhysicsDemoMenuView>(_renderer.get());
}

std::unique_ptr<PhysicsDemoView> ViewBuilder::buildPhysicsDemoView() {
    return std::make_unique<PhysicsDemoView>(
        _renderer.get(), _engine.get(), _inputController.get(), _resourceManager.get()
    );
}