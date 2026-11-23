#pragma once

#include "appMode.h"
#include "editorRenderer.h"
#include "frameTime.h"
#undef Status
#include "imgui.h"
#include <algorithm>
#include <imgui_internal.h>

namespace vax::editor {
class ViewManager;

class View {
  public:
    View(vax::gfx::EditorRenderer& editorRenderer)
        : _editorRenderer(editorRenderer) {};

    virtual ~View() = default;

    virtual void updateUI(const vax::time::FrameTime& frameTime) = 0;

    virtual void update(const vax::time::FrameTime& frameTime) = 0;

    virtual bool draw(const vax::time::FrameTime& frameTime) { return _editorRenderer.get().render(nullptr, frameTime); }

    virtual void viewWillShow() { _editorRenderer.get().renderPassGraphMode.set(gfx::RenderPassGraphMode::UI_ONLY); }

    virtual app::AppMode getAppMode() { return app::AppMode::EventDriven; }

    void linkViewManager(ViewManager* viewManager) { _viewManager = viewManager; }

  protected:
    std::reference_wrapper<vax::gfx::EditorRenderer> _editorRenderer;
    ViewManager* _viewManager = nullptr;

    bool _beginInMainWindow(const char* name, ImGuiWindowFlags flags = 0) {
        const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowViewport(mainViewport->ID);

        if (const ImGuiWindow* window = ImGui::FindWindowByName(name)) {
            ImVec2 minPosition = mainViewport->WorkPos;
            ImVec2 maxPosition = ImVec2(
                std::max(minPosition.x, mainViewport->WorkPos.x + mainViewport->WorkSize.x - window->Size.x),
                std::max(minPosition.y, mainViewport->WorkPos.y + mainViewport->WorkSize.y - window->Size.y)
            );
            ImVec2 clamped = ImVec2(
                std::clamp(window->Pos.x, minPosition.x, maxPosition.x),
                std::clamp(window->Pos.y, minPosition.y, maxPosition.y)
            );
            if (clamped.x != window->Pos.x || clamped.y != window->Pos.y) {
                ImGui::SetNextWindowPos(clamped);
            }
        }
        return ImGui::Begin(name, nullptr, flags | ImGuiWindowFlags_NoDocking);
    }
};
} // namespace vax::editor