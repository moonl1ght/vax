#include "menuView.h"
#undef Status
#include "imgui.h"
#include "physicsDemoMenu.h"
#include "viewManager.h"

using namespace vax::ui;
using namespace vax;

void MenuView::update(const vax::engine::FrameTime& frameTime) {
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
        _showGridWorldDemo();
    }
    ImGui::Spacing();
    if (ImGui::Button("Physics Engine Demo", ImVec2(-1, 55))) {
        _showPhysicsEngineDemo();
    }
    ImGui::End();
    DebugView::update(frameTime);
}

void MenuView::_showGridWorldDemo() {
    auto viewBuilder = _viewManager->viewBuilder();
    if (_viewManager) {
        _viewManager->pushView(viewBuilder.buildGridWorldView());
    }
}

void MenuView::_showPhysicsEngineDemo() {
    auto viewBuilder = _viewManager->viewBuilder();
    if (_viewManager) {
        _viewManager->pushView(viewBuilder.buildPhysicsDemoMenuView());
    }
}