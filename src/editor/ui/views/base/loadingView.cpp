#include "loadingView.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>

using namespace vax::editor;

void LoadingView::updateUI(const vax::time::FrameTime& frameTime) {
    if (!_isShown) {
        return;
    }
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 center =
        ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f, viewport->WorkPos.y + viewport->WorkSize.y * 0.5f);

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 0), ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    if (!ImGui::Begin("Loading", nullptr, flags)) {
        ImGui::End();
        return;
    }

    _showIndicator();
    if (_progress) {
        ImGui::Spacing();
        _showProgress();
    }
    ImGui::End();
}

void LoadingView::_showIndicator() const {
    int dotCount = static_cast<int>(std::fmod(ImGui::GetTime() * 3.0, 4.0));
    std::string text = _message + std::string(dotCount, '.');
    ImGui::Text("%s", text.c_str());
}

void LoadingView::_showProgress() const {
    float progress = std::clamp(*_progress, 0.0f, 1.0f);
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
}
