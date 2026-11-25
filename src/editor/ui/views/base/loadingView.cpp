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

    _showSpinner();
    ImGui::Spacing();
    _showIndicator();
    if (_progress) {
        ImGui::Spacing();
        _showProgress();
    }
    ImGui::End();
}

void LoadingView::_showSpinner() const {
    constexpr float radius = 18.0f;
    constexpr float thickness = 4.0f;
    constexpr int segments = 32;
    constexpr float speed = 6.0f;
    constexpr float sweep = IM_PI * 1.5f;

    const float width = ImGui::GetContentRegionAvail().x;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 center(origin.x + width * 0.5f, origin.y + radius + thickness);
    ImGui::Dummy(ImVec2(width, (radius + thickness) * 2.0f));

    const float start = static_cast<float>(ImGui::GetTime()) * speed;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->PathClear();
    for (int i = 0; i <= segments; ++i) {
        float angle = start + sweep * static_cast<float>(i) / static_cast<float>(segments);
        drawList->PathLineTo(ImVec2(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius));
    }
    drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_ButtonActive), ImDrawFlags_None, thickness);
}

void LoadingView::_showIndicator() const {
    int dotCount = static_cast<int>(std::fmod(ImGui::GetTime() * 3.0, 4.0));
    std::string text = _message + std::string(dotCount, '.') + std::string(3 - dotCount, ' ');
    float textWidth = ImGui::CalcTextSize(text.c_str()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - textWidth) * 0.5f);
    ImGui::Text("%s", text.c_str());
}

void LoadingView::_showProgress() const {
    float progress = std::clamp(*_progress, 0.0f, 1.0f);
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
}
