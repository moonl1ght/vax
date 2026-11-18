#include "viewBuilder.h"
#include "physicsDemoMenu.h"
#include "startingView.h"

using namespace vax::ui;

std::unique_ptr<StartingView> ViewBuilder::buildStartingView() {
    return std::make_unique<StartingView>(_renderingEngine.get());
}

std::unique_ptr<GridWorldView> ViewBuilder::buildGridWorldView() {
    return std::make_unique<GridWorldView>(_windowController.get(), _renderingEngine.get(), _inputController.get());
}

std::unique_ptr<PhysicsDemoMenuView> ViewBuilder::buildPhysicsDemoMenuView() {
    return std::make_unique<PhysicsDemoMenuView>(_renderingEngine.get());
}

std::unique_ptr<PhysicsDemoView> ViewBuilder::buildPhysicsDemoView() {
    return std::make_unique<PhysicsDemoView>(_renderingEngine.get(), _inputController.get());
}