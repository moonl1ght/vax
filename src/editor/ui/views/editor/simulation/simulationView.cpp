#include "simulationView.h"
#include "IconsFontAwesome6.h"
#include "closeButton.h"
#include <imgui.h>

using namespace vax::editor;

void SimulationView::updateUI(const vax::time::FrameTime& frameTime) {
    _simSettingsView->updateUI(frameTime);
    _simStatsView->updateUI(frameTime);

    if (!_isShown) {
        return;
    }
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowViewport(mainViewport->ID);
    ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 10.0f, mainViewport->WorkPos.y + 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(WIDTH, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.33f, 0.55f, 0.95f, 1.00f));
    const bool isOpen = ImGui::Begin("Simulation", &_isShown, flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    if (!isOpen) {
        ImGui::End();
        return;
    }

    CloseButton::topCorner(_isShown);

    _showTime();
    _showControls();
    ImGui::Spacing();
    _showWindowButtons();

    _height = ImGui::GetWindowHeight();
    ImGui::End();
}

void SimulationView::_showTime() const {
    const long long minutes = 0;
    const long long seconds = 0;
    const long long millis = 0;

    ImGui::Text(ICON_FA_CLOCK " %02lld:%02lld.%03lld", minutes, seconds, millis);
    ImGui::SameLine();
    ImGui::TextDisabled("%s", "Stopped");
}

void SimulationView::_showControls() {
    const ImGuiStyle& style = ImGui::GetStyle();
    const float buttonWidth = (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x * 2.0f) / 3.0f;

    if (ImGui::Button(ICON_FA_PLAY "##start", ImVec2(buttonWidth, 0.0f))) {
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PAUSE "##pause", ImVec2(buttonWidth, 0.0f))) {
    }

    // ImGui::SameLine();
    // if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##restart", ImVec2(buttonWidth, 0.0f))) {
    // }
}

void SimulationView::_showWindowButtons() {
    const float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

    SimSettingsView* simSettingsView = _simSettingsView.get();
    if (ImGui::Button(ICON_FA_GEAR "##settings", ImVec2(buttonWidth, 0.0f))) {
        simSettingsView->setShown(!simSettingsView->isShown());
    }

    ImGui::SameLine();
    SimStatsView* simStatsView = _simStatsView.get();
    if (ImGui::Button(ICON_FA_CHART_LINE "##stats", ImVec2(buttonWidth, 0.0f))) {
        simStatsView->setShown(!simStatsView->isShown());
    }
}
