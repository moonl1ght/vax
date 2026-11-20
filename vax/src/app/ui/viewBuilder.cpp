#include "viewBuilder.h"
#include "physicsDemoMenu.h"
#include "startingView.h"

using namespace vax::ui;

std::unique_ptr<StartingView> ViewBuilder::buildStartingView() {
    return std::make_unique<StartingView>(_renderEngine.get());
}

std::unique_ptr<GridWorldView> ViewBuilder::buildGridWorldView() {
    return std::make_unique<GridWorldView>(_windowController.get(), _renderEngine.get(), _inputController.get());
}

std::unique_ptr<PhysicsDemoMenuView> ViewBuilder::buildPhysicsDemoMenuView() {
    return std::make_unique<PhysicsDemoMenuView>(_renderEngine.get());
}

std::unique_ptr<PhysicsDemoView> ViewBuilder::buildPhysicsDemoView() {
    return std::make_unique<PhysicsDemoView>(_renderEngine.get(), _inputController.get());
}