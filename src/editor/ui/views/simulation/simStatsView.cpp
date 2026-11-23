#include "simStatsView.h"
#include <imgui.h>

using namespace vax::editor;

void SimStatsView::updateUI(const vax::time::FrameTime& frameTime) {
    if (!_isShown) {
        return;
    }
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(mainViewport->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 320), ImGuiCond_FirstUseEver);

    ImGuiWindowClass windowClass;
    windowClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
    ImGui::SetNextWindowClass(&windowClass);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
    if (ImGui::Begin("Simulation stats", &_isShown, flags)) {
        _showContents();
    }
    ImGui::End();
}

void SimStatsView::_showContents() { ImGui::TextDisabled("No simulation stats yet"); }
