#pragma once

#include "editorRenderer.h"
#include "view.h"

namespace vax::editor {

class SimSettingsView final : public View {
  public:
    SimSettingsView(vax::gfx::EditorRenderer& editorRenderer)
        : View(editorRenderer) {}

    ~SimSettingsView() override = default;

    SimSettingsView(const SimSettingsView& other) = delete;
    SimSettingsView(SimSettingsView&& other) noexcept = delete;
    SimSettingsView& operator=(const SimSettingsView& other) = delete;
    SimSettingsView& operator=(SimSettingsView&& other) noexcept = delete;

    void update(const vax::time::FrameTime& frameTime) override {}

    void updateUI(const vax::time::FrameTime& frameTime) override;

    bool isShown() const noexcept { return _isShown; }

    void setShown(bool isShown) noexcept { _isShown = isShown; }

  private:
    bool _isShown = false;

    void _showContents();
};

} // namespace vax::editor
