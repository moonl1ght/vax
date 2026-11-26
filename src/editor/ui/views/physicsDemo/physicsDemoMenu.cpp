#include "physicsDemoMenu.h"
#undef Status
#include "imgui.h"
#include "mainView.h"
#include "viewManager.h"

using namespace vax::editor;

void PhysicsDemoMenuView::updateUI(const vax::time::FrameTime& frameTime) {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 0), ImGuiCond_Always);
    _beginInMainWindow(
        "VAX",
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
    );
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
    if (ImGui::Button("Show Sphere Demo", ImVec2(-1, 55))) {
        _showSphereDemo();
    }
    ImGui::End();

    MenuView::updateUI(frameTime);
}

void PhysicsDemoMenuView::_showSphereDemo() {
    auto viewBuilder = _viewManager->viewBuilder();
    if (_viewManager) {
        _viewManager->pushView(viewBuilder.buildPhysicsSphereDemoView());
    }
}
