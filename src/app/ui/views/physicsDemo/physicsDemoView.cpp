#include "physicsDemoView.h"
#include "sceneLoader.h"
#include "viewManager.h"

using namespace vax::ui;

void PhysicsDemoView::updateUI(const vax::engine::FrameTime& frameTime) { SceneView::updateUI(frameTime); }

void PhysicsDemoView::load(vax::vk::Engine& engine, vax::InputController& inputController) {
    auto sceneLoader = vax::engine::SceneLoader(engine);

    auto sceneWorldPair = sceneLoader.load(RES_PATH("scenes/physics_demo_scene.json"));

    if (!sceneWorldPair.has_value()) {
        _logger.error("Failed to load physics demo scene!");
        return;
    }

    _controller =
        std::make_unique<vax::MainSceneController>(std::move(sceneWorldPair->first), std::move(sceneWorldPair->second));

    _controller->getScene().resize();
    inputController.addObserver(&_controller->getScene());
}