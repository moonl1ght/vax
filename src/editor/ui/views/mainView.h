#pragma once

#include "debugView.h"

namespace vax::editor {
class MainView final : public DebugView {
  public:
    MainView(vax::gfx::EditorRenderer_old& editorRenderer)
        : DebugView(editorRenderer, nullptr) {};

    ~MainView() override = default;

    MainView(const MainView& other) = delete;
    MainView& operator=(const MainView& other) = delete;
    MainView(MainView&& other) noexcept = delete;
    MainView& operator=(MainView&& other) noexcept = delete;

    void updateUI(const vax::time::FrameTime& frameTime) override;

  protected:
    bool _isClosable() const override { return false; }

  private:
    void _showGridWorldDemo();

    void _showPhysicsEngineDemo();
};
} // namespace vax::editor