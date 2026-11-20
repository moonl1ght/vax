#pragma once

#include "debugView.h"

namespace vax::ui {
class StartingView final : public DebugView {
  public:
    StartingView(vax::gfx::RenderEngine& renderEngine)
        : DebugView(renderEngine) {};

    ~StartingView() override = default;

    StartingView(const StartingView& other) = delete;
    StartingView& operator=(const StartingView& other) = delete;
    StartingView(StartingView&& other) noexcept = delete;
    StartingView& operator=(StartingView&& other) noexcept = delete;

    void updateUI(const vax::gfx::FrameTime& frameTime) override;

  protected:
    bool _isClosable() const override { return false; }

  private:
    void _showGridWorldDemo();

    void _showPhysicsEngineDemo();
};
} // namespace vax::ui