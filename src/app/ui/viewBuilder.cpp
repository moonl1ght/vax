#include "viewBuilder.h"
#include "menuView.h"
#include "physicsDemoMenu.h"

using namespace vax::ui;

std::unique_ptr<MenuView> ViewBuilder::buildMenuView() { return std::make_unique<MenuView>(_renderer.get()); }

std::unique_ptr<GridWorldView> ViewBuilder::buildGridWorldView() {
    auto gridWorldView = std::make_unique<GridWorldView>(_uiEngine.get(), _windowController.get(), _renderer.get());
    gridWorldView->load(_engine.get(), _inputController.get());
    return gridWorldView;
}

std::unique_ptr<PhysicsDemoMenuView> ViewBuilder::buildPhysicsDemoMenuView() {
    return std::make_unique<PhysicsDemoMenuView>(_renderer.get());
}

std::unique_ptr<PhysicsDemoView> ViewBuilder::buildPhysicsDemoView() {
    auto physicsDemoView = std::make_unique<PhysicsDemoView>(_renderer.get());
    physicsDemoView->load(_engine.get(), _inputController.get());
    return physicsDemoView;
}