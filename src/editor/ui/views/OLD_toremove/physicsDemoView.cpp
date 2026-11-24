#include "physicsDemoView.h"
#include "paths.h"
#include "sceneLoader_old.h"
#include "viewManager.h"
#include "renderEngine.h"

using namespace vax::editor;

void PhysicsDemoView::updateUI(const vax::time::FrameTime& frameTime) { SceneView::updateUI(frameTime); }

bool PhysicsDemoView::_load() {
    auto sceneLoader = vax::gfx::SceneLoader(
        _rendererEngine.get().vkEngine(),
        *_rendererEngine.get().vkEngine().pipelineManager,
        _rendererEngine.get().resourceManager(),
        _windowController.get()
    );

    auto sceneWorldPair = sceneLoader.load(RES_PATH("scenes/physics_demo_scene.json"));

    if (!sceneWorldPair.has_value()) {
        _logger.error("Failed to load physics demo scene!");
        return false;
    }

    _controller =
        std::make_unique<app::MainSceneController>(std::move(sceneWorldPair->first), std::move(sceneWorldPair->second));

    _controller->load();

    _controller->getScene().resize();
    _inputController.get().addObserver(&_controller->getScene());
    return true;
}
