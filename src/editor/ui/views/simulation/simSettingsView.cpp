#include "simSettingsView.h"
#include <imgui.h>

using namespace vax::editor;

void SimSettingsView::updateUI(const vax::gfx::FrameTime& frameTime) {
    if (!_isShown) {
        return;
    }
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(mainViewport->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 360), ImGuiCond_FirstUseEver);

    ImGuiWindowClass windowClass;
    windowClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
    ImGui::SetNextWindowClass(&windowClass);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
    if (ImGui::Begin("Simulation settings", &_isShown, flags)) {
        _showContents();
    }
    ImGui::End();
}

void SimSettingsView::_showContents() { ImGui::TextDisabled("No simulation settings yet"); }
