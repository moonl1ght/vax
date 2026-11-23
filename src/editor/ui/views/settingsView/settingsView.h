#pragma once

#include "editorRenderer.h"
#include "view.h"

namespace vax::editor {

class SettingsView final : public View {
  public:
    SettingsView(vax::gfx::EditorRenderer& editorRenderer)
        : View(editorRenderer) {}

    ~SettingsView() override = default;

    SettingsView(const SettingsView& other) = delete;
    SettingsView(SettingsView&& other) noexcept = delete;
    SettingsView& operator=(const SettingsView& other) = delete;
    SettingsView& operator=(SettingsView&& other) noexcept = delete;

    void update(const vax::time::FrameTime& frameTime) override {}

    void updateUI(const vax::time::FrameTime& frameTime) override;

    bool isShown() const noexcept { return _isShown; }

    void setShown(bool isShown) noexcept { _isShown = isShown; }

  private:
    bool _isShown = false;

    void _showContents();
};

} // namespace vax::editor
