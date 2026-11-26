    #include "mainView.h"
#undef Status
#include "imgui.h"
#include "physicsDemoMenu.h"
#include "viewManager.h"

using namespace vax::editor;
using namespace vax;

void MainView::updateUI(const vax::time::FrameTime& frameTime) {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 0), ImGuiCond_Always);
    _beginInMainWindow(
        "VAX",
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
    MenuView::updateUI(frameTime);
}

void MainView::_showGridWorldDemo() {
    auto viewBuilder = _viewManager->viewBuilder();
    if (_viewManager) {
        _viewManager->pushView(viewBuilder.buildGridWorldView());
    }
}

void MainView::_showPhysicsEngineDemo() {
    auto viewBuilder = _viewManager->viewBuilder();
    if (_viewManager) {
        _viewManager->pushView(viewBuilder.buildPhysicsDemoMenuView());
    }
}