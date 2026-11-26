#pragma once

#include "editorRenderer_old.h"
#include "simSettingsView.h"
#include "simStatsView.h"
#include "view.h"

namespace vax::editor {

class SimulationView final : public View {
  public:
    SimulationView(vax::gfx::EditorRenderer_old& editorRenderer, vax::gfx::EditorRenderer* editorRenderer_new)
        : View(editorRenderer, editorRenderer_new) {
        _simSettingsView = std::make_unique<SimSettingsView>(editorRenderer, editorRenderer_new);
        _simStatsView = std::make_unique<SimStatsView>(editorRenderer, editorRenderer_new);
    }

    ~SimulationView() override = default;

    void updateUI(const vax::time::FrameTime& frameTime) override;

    bool isShown() const noexcept { return _isShown; }

    void setShown(bool isShown) noexcept { _isShown = isShown; }

    float height() const noexcept { return _isShown ? _height : 0.0f; }

  private:
    static constexpr float WIDTH = 500.0f;

    std::unique_ptr<SimSettingsView> _simSettingsView;
    std::unique_ptr<SimStatsView> _simStatsView;

    bool _isShown = false;
    float _height = 0.0f;

    void _showControls();
    void _showTime() const;
    void _showWindowButtons();
};
} // namespace vax::editor
