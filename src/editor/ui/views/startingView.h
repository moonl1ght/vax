#pragma once

#include "debugView.h"

namespace vax::editor {
class StartingView final : public DebugView {
  public:
    StartingView(vax::gfx::EditorRenderer& editorRenderer)
        : DebugView(editorRenderer) {};

    ~StartingView() override = default;

    StartingView(const StartingView& other) = delete;
    StartingView& operator=(const StartingView& other) = delete;
    StartingView(StartingView&& other) noexcept = delete;
    StartingView& operator=(StartingView&& other) noexcept = delete;

    void updateUI(const vax::time::FrameTime& frameTime) override;

  protected:
    bool _isClosable() const override { return false; }

  private:
    void _showGridWorldDemo();

    void _showPhysicsEngineDemo();
};
} // namespace vax::editor