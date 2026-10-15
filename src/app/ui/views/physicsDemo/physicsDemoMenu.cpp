#include "physicsDemoMenu.h"
#undef Status
#include "imgui.h"
#include "startingView.h"
#include "viewManager.h"

using namespace vax::ui;

void PhysicsDemoMenuView::updateUI(const vax::engine::FrameTime& frameTime) {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 0), ImGuiCond_Always);
    _beginInMainWindow(
        "VAX",
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
    );
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
    if (ImGui::Button("Show Simple Demo", ImVec2(-1, 55))) {
        _showSimpleDemo();
    }
    ImGui::End();

    DebugView::updateUI(frameTime);
}

void PhysicsDemoMenuView::_showSimpleDemo() {
    auto viewBuilder = _viewManager->viewBuilder();
    if (_viewManager) {
        _viewManager->pushView(viewBuilder.buildPhysicsDemoView());
    }
}
