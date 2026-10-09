#include "menuView.h"
#undef Status
#include "imgui.h"
#include "physicsDemoMenu.h"
#include "viewManager.h"

using namespace vax::ui;
using namespace vax;

void MenuView::update(const vax::engine::FrameTime& frameTime) {
    auto action = _popPendingAction();
    if (action) {
        _handleAction(action.value());
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowSize(ImVec2(480, 220), ImGuiCond_Always);
    ImGui::Begin(
        "VAX",
        nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
    );
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
    if (ImGui::Button("Show Grid World Demo", ImVec2(-1, 55))) {
        _pendingAction = Action::SHOW_GRID_WORLD_DEMO;
    }
    ImGui::Spacing();
    if (ImGui::Button("Train Q Learning", ImVec2(-1, 55))) {
        _pendingAction = Action::TRAIN_Q_LEARNING;
    }
    ImGui::Spacing();
    if (ImGui::Button("Physics Engine Demo", ImVec2(-1, 55))) {
        _pendingAction = Action::SHOW_PHYSICS_ENGINE_DEMO;
    }
    ImGui::End();

    if (_trainingView) {
        _trainingView->update(frameTime);
    }
    DebugView::update(frameTime);
}

void MenuView::_handleAction(Action action) {
    auto viewBuilder = _viewManager->viewBuilder();
    switch (action) {
    case Action::SHOW_GRID_WORLD_DEMO:
        if (_viewManager) {
            _viewManager->setRootView(viewBuilder.buildGridWorldView());
        }
        break;
    case Action::TRAIN_Q_LEARNING:
        _trainingView = std::make_unique<TrainingView>(_renderer.get());
        _trainingView->startTraining();
        break;
    case Action::SHOW_PHYSICS_ENGINE_DEMO:
        if (_viewManager) {
            _viewManager->setRootView(viewBuilder.buildPhysicsDemoMenuView());
        }
        break;
    }
}