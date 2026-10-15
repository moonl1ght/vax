#pragma once

#include "frameProfiler.h"
#include "statsView.h"
#include "view.h"
#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>

namespace vax::ui {
class DebugView : public View {
  public:
    DebugView(vax::engine::Renderer& renderer)
        : View(renderer) {
        _statsView = std::make_unique<StatsView>(renderer);
        _frameProfiler = std::make_unique<vax::FrameProfiler>();
        _statsView->linkFrameProfiler(_frameProfiler.get());
    }

    ~DebugView() override { _renderer.get().unlinkFrameProfiler(_frameProfiler.get()); }

    void updateUI(const vax::engine::FrameTime& frameTime) override {
        _showMenuBar();
        _statsView->updateUI(frameTime);
    }

    void update(const vax::engine::FrameTime& frameTime) override { _statsView->update(frameTime); }

    void viewWillShow() override {
        _renderer.get().linkFrameProfiler(_frameProfiler.get());
        View::viewWillShow();
    }

  protected:
    std::unique_ptr<StatsView> _statsView;
    std::unique_ptr<vax::FrameProfiler> _frameProfiler;

    virtual void _showMenuBarItems() {}

    virtual bool _isClosable() const { return true; }

    virtual bool _isCloseEnabled() const { return true; }

    virtual void _onClose();

  private:
    float _menuBarRightBlockWidth = 0.0f;

    void _showMenuBar() {
        if (!ImGui::BeginMainMenuBar()) {
            return;
        }
        _showMenuBarItems();

        const float rightBlockX = ImGui::GetWindowWidth() - _menuBarRightBlockWidth - ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), rightBlockX));
        const float rightBlockStartX = ImGui::GetCursorScreenPos().x;
        if (ImGui::MenuItem("Info", nullptr, _statsView->isShown())) {
            _statsView->setShown(!_statsView->isShown());
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Settings")) {
            // TODO: settings
        }
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
} // namespace vax::ui