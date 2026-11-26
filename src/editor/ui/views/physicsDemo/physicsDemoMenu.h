#pragma once

#include "debugView.h"

namespace vax::editor {
class PhysicsDemoMenuView final : public DebugView {
  public:
    PhysicsDemoMenuView(vax::gfx::EditorRenderer_old& editorRenderer)
        : DebugView(editorRenderer, nullptr) {}

    ~PhysicsDemoMenuView() override = default;

    PhysicsDemoMenuView(const PhysicsDemoMenuView& other) = delete;
    PhysicsDemoMenuView& operator=(const PhysicsDemoMenuView& other) = delete;
    PhysicsDemoMenuView(PhysicsDemoMenuView&& other) noexcept = delete;
    PhysicsDemoMenuView& operator=(PhysicsDemoMenuView&& other) noexcept = delete;

    void updateUI(const vax::time::FrameTime& frameTime) override;

  private:
    void _showSphereDemo();
};
} // namespace vax::editor