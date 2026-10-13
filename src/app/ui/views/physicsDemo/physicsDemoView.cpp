#include "physicsDemoView.h"
#include "sceneLoader.h"
#include "viewManager.h"

using namespace vax::ui;

void PhysicsDemoView::updateUI(const vax::engine::FrameTime& frameTime) {
    ImGui::SetNextWindowSize(ImVec2(380, 360), ImGuiCond_FirstUseEver);
    ImGui::Begin("Physics demo");
    if (ImGui::Button("Back", ImVec2(-1, 55))) {
        _viewManager->popView();
    }
    ImGui::End();
    SceneView::updateUI(frameTime);
}

void PhysicsDemoView::load(vax::vk::Engine& engine, vax::InputController& inputController) {
    auto sceneLoader = vax::engine::SceneLoader(engine);

    _scene = sceneLoader.load(RES_PATH("scenes/physics_demo_scene.json"));
    _scene->drawableScene().resize();
    inputController.addObserver(&_scene->drawableScene());
}