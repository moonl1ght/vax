#pragma once

#include "editorRenderer_old.h"
#include "view.h"

namespace vax::editor {
class FullStatsView final : public View {
  public:
    FullStatsView(vax::gfx::EditorRenderer_old& editorRenderer)
        : View(editorRenderer) {}

    ~FullStatsView() override = default;

    FullStatsView(const FullStatsView& other) = delete;
    FullStatsView(FullStatsView&& other) noexcept = delete;
    FullStatsView& operator=(const FullStatsView& other) = delete;
    FullStatsView& operator=(FullStatsView&& other) noexcept = delete;

    void update(const vax::time::FrameTime& frameTime) override {}

    void updateUI(const vax::time::FrameTime& frameTime) override;

    bool isShown() const noexcept { return _isShown; }

    void setShown(bool isShown) noexcept { _isShown = isShown; }

  private:
    bool _isShown = false;

    void _showContents();
};
} // namespace vax::editor
