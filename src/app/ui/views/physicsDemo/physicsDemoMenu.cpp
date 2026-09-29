#include "physicsDemoMenu.h"
#undef Status
#include "imgui.h"
#include "menuView.h"
#include "viewManager.h"

using namespace vax::ui;

void PhysicsDemoMenuView::update(const vax::engine::FrameTime&) {
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
    if (ImGui::Button("Show Simple Demo", ImVec2(-1, 55))) {
        _pendingAction = Action::SHOW_SIMPLE_DEMO;
    }
    ImGui::Spacing();
    if (ImGui::Button("Back", ImVec2(-1, 55))) {
        _pendingAction = Action::GO_TO_MAIN_MENU;
    }
    ImGui::End();
}

void PhysicsDemoMenuView::_handleAction(Action action) {
    switch (action) {
    case Action::GO_TO_MAIN_MENU:
        if (_viewManager) {
            _viewManager->setRootView(_viewBuilder.get().buildMenuView());
        }
        break;
    case Action::SHOW_SIMPLE_DEMO:
        break;
    }
}