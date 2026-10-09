#include "physicsDemoView.h"
#include "sceneLoader.h"

using namespace vax::ui;

void PhysicsDemoView::update(const vax::engine::FrameTime& frameTime) { SceneView::update(frameTime); }

void PhysicsDemoView::render(const vax::engine::FrameTime& frameTime) { SceneView::drawScene(frameTime); }

void PhysicsDemoView::load(vax::vk::Engine& engine, vax::InputController& inputController) {
    _renderer.get().linkFrameProfiler(_frameProfiler.get());


    auto sceneLoader = vax::engine::SceneLoader(engine);

    _scene = sceneLoader.load(RES_PATH("scenes/physics_demo_scene.json"));
    _scene->drawableScene().resize();
    inputController.addObserver(&_scene->drawableScene());
}