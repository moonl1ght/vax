#pragma once

#include "IconsFontAwesome6.h"
#include "frameProfiler.h"
#include "renderer.h"
#include "settingsView.h"
#include "simSettingsView.h"
#include "simStatsView.h"
#include "simulationView.h"
#include "statsView.h"
#include "view.h"
#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>

namespace vax::editor {
class DebugView : public View {
  public:
    DebugView(vax::gfx::Renderer& renderer)
        : View(renderer) {
        _statsView = std::make_unique<StatsView>(renderer);
        _settingsView = std::make_unique<SettingsView>(renderer);
        _simulationView = std::make_unique<SimulationView>(renderer);
        _frameProfiler = std::make_unique<dev::FrameProfiler>();
        _statsView->linkFrameProfiler(_frameProfiler.get());
    }

    ~DebugView() override { _renderer.get().unlinkFrameProfiler(_frameProfiler.get()); }

    void updateUI(const vax::gfx::FrameTime& frameTime) override {
        _showMenuBar();
        _showDockSpace();
        _simulationView->updateUI(frameTime);
        _statsView->updateUI(frameTime);
        _settingsView->updateUI(frameTime);
    }

    void update(const vax::gfx::FrameTime& frameTime) override {
        _statsView->update(frameTime);
        _simulationView->update(frameTime);
    }

    void viewWillShow() override {
        _renderer.get().linkFrameProfiler(_frameProfiler.get());
        View::viewWillShow();
    }

  protected:
    std::unique_ptr<StatsView> _statsView;
    std::unique_ptr<SettingsView> _settingsView;
    std::unique_ptr<SimulationView> _simulationView;
    std::unique_ptr<dev::FrameProfiler> _frameProfiler;

    virtual void _showMenuBarItems() {}

    virtual bool _isClosable() const { return true; }

    virtual bool _isCloseEnabled() const { return true; }

    virtual void _onClose();

  private:
    static constexpr float DOCK_RIGHT_RATIO = 0.28f;

    float _menuBarRightBlockWidth = 0.0f;

    void _showDockSpace() {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImGuiID dockSpaceId = ImGui::GetID("EditorDockSpace");
        constexpr ImGuiDockNodeFlags flags =
            ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingOverCentralNode;

        if (ImGui::DockBuilderGetNode(dockSpaceId) == nullptr) {
            ImGui::DockBuilderAddNode(dockSpaceId, flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockSpaceId, viewport->WorkSize);

            ImGui::DockBuilderFinish(dockSpaceId);
        }

        ImGui::DockSpaceOverViewport(dockSpaceId, viewport, flags);
    }

    void _showMenuBar() {
        if (!ImGui::BeginMainMenuBar()) {
            return;
        }

        if (ImGui::MenuItem("Simulation", nullptr, _simulationView->isShown())) {
            _simulationView->setShown(!_simulationView->isShown());
        }
        ImGui::Separator();
        _showMenuBarItems();

        const float rightBlockX = ImGui::GetWindowWidth() - _menuBarRightBlockWidth - ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), rightBlockX));
        const float rightBlockStartX = ImGui::GetCursorScreenPos().x;
        if (ImGui::MenuItem("Stats", nullptr, _statsView->isShown())) {
            _statsView->setShown(!_statsView->isShown());
        }
        ImGui::Separator();
        if (ImGui::MenuItem(ICON_FA_GEAR "##settings", nullptr, _settingsView->isShown())) {
            _settingsView->setShown(!_settingsView->isShown());
        }
        ImGui::SetItemTooltip("Settings");
        if (_isClosable()) {
            ImGui::Separator();
            if (ImGui::MenuItem("Close", nullptr, false, _isCloseEnabled())) {
                _onClose();
            }
        }
        _menuBarRightBlockWidth = ImGui::GetItemRectMax().x - rightBlockStartX;
        ImGui::EndMainMenuBar();
    }
};
} // namespace vax::editor