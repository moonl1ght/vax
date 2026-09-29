#include "viewBuilder.h"
#include "menuView.h"
#include "physicsDemoMenu.h"

using namespace vax::ui;

std::unique_ptr<MenuView> ViewBuilder::buildMenuView() { return std::make_unique<MenuView>(*this, _renderer.get()); }

std::unique_ptr<RoverView> ViewBuilder::buildRoverView() {
    auto roverView = std::make_unique<RoverView>(_uiEngine.get(), _windowController.get(), _renderer.get());
    roverView->load(_engine.get(), _inputController.get());
    return roverView;
}

std::unique_ptr<PhysicsDemoMenuView> ViewBuilder::buildPhysicsDemoMenuView() {
    return std::make_unique<PhysicsDemoMenuView>(*this, _uiEngine.get(), _renderer.get());
}