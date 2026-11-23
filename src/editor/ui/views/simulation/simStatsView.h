#pragma once

#include "renderer.h"
#include "view.h"

namespace vax::editor {
class SimStatsView final : public View {
  public:
    SimStatsView(vax::gfx::Renderer& renderer)
        : View(renderer) {}

    ~SimStatsView() override = default;

    SimStatsView(const SimStatsView& other) = delete;
    SimStatsView(SimStatsView&& other) noexcept = delete;
    SimStatsView& operator=(const SimStatsView& other) = delete;
    SimStatsView& operator=(SimStatsView&& other) noexcept = delete;

    void update(const vax::gfx::FrameTime& frameTime) override {}

    void updateUI(const vax::gfx::FrameTime& frameTime) override;

    bool isShown() const noexcept { return _isShown; }

    void setShown(bool isShown) noexcept { _isShown = isShown; }

  private:
    bool _isShown = false;

    void _showContents();
};
} // namespace vax::editor
