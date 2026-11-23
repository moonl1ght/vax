#include "fullStatsView.h"
#include <imgui.h>

using namespace vax::editor;

void FullStatsView::updateUI(const vax::gfx::FrameTime& frameTime) {
    if (!_isShown) {
        return;
    }
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(mainViewport->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);

    ImGuiWindowClass windowClass;
    windowClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
    ImGui::SetNextWindowClass(&windowClass);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
    if (ImGui::Begin("Full stats", &_isShown, flags)) {
        _showContents();
    }
    ImGui::End();
}

void FullStatsView::_showContents() { ImGui::TextDisabled("No detailed stats yet"); }
