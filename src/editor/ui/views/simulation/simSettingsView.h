#pragma once

#include "renderer.h"
#include "view.h"

namespace vax::editor {

class SimSettingsView final : public View {
  public:
    SimSettingsView(vax::gfx::Renderer& renderer)
        : View(renderer) {}

    ~SimSettingsView() override = default;

    SimSettingsView(const SimSettingsView& other) = delete;
    SimSettingsView(SimSettingsView&& other) noexcept = delete;
    SimSettingsView& operator=(const SimSettingsView& other) = delete;
    SimSettingsView& operator=(SimSettingsView&& other) noexcept = delete;

    void update(const vax::gfx::FrameTime& frameTime) override {}

    void updateUI(const vax::gfx::FrameTime& frameTime) override;

    bool isShown() const noexcept { return _isShown; }

    void setShown(bool isShown) noexcept { _isShown = isShown; }

  private:
    bool _isShown = false;

    void _showContents();
};

} // namespace vax::editor
