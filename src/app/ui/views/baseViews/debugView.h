#pragma once

#include "frameProfiler.h"
#include "statsView.h"
#include "view.h"

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

    void update(const vax::engine::FrameTime& frameTime) override {
        _statsView->update(frameTime);
    }

  protected:
    std::unique_ptr<StatsView> _statsView;
    std::unique_ptr<vax::FrameProfiler> _frameProfiler;
};
} // namespace vax::ui