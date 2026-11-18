#include "physicsDemoView.h"
#include "paths.h"
#include "sceneLoader.h"
#include "viewManager.h"

using namespace vax::ui;

void PhysicsDemoView::updateUI(const vax::engine::FrameTime& frameTime) { SceneView::updateUI(frameTime); }

bool PhysicsDemoView::_load() {
    auto sceneLoader = vax::engine::SceneLoader(_vkEngine.get(), _resourceManager.get());

    auto sceneWorldPair = sceneLoader.load(RES_PATH("scenes/physics_demo_scene.json"));

    if (!sceneWorldPair.has_value()) {
        _logger.error("Failed to load physics demo scene!");
        return false;
    }

    _controller =
        std::make_unique<vax::MainSceneController>(std::move(sceneWorldPair->first), std::move(sceneWorldPair->second));

    _controller->load();

    _controller->getScene().resize();
    _inputController.get().addObserver(&_controller->getScene());
    return true;
}
