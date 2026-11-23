#pragma once

#include <imgui.h>
#include <imgui_internal.h>

namespace vax::editor {

struct CloseButton final {

    static void topCorner(bool& isShown) {
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImVec2 windowPos = ImGui::GetWindowPos();
        const ImVec2 buttonPos = ImVec2(
            windowPos.x + ImGui::GetWindowWidth() - ImGui::GetFontSize() - style.WindowPadding.x * 0.5f,
            windowPos.y + style.WindowPadding.y * 0.5f
        );
        if (ImGui::CloseButton(ImGui::GetID("##close"), buttonPos)) {
            isShown = false;
        }
    }
};

} // namespace vax::editor