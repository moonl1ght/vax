#pragma once

#include "debugView.h"
#include "viewBuilder.h"

namespace vax::ui {
class PhysicsDemoMenuView final : public DebugView {
  public:
    PhysicsDemoMenuView(vax::engine::Renderer& renderer)
        : DebugView(renderer) {}

    ~PhysicsDemoMenuView() override = default;

    PhysicsDemoMenuView(const PhysicsDemoMenuView& other) = delete;
    PhysicsDemoMenuView& operator=(const PhysicsDemoMenuView& other) = delete;
    PhysicsDemoMenuView(PhysicsDemoMenuView&& other) noexcept = delete;
    PhysicsDemoMenuView& operator=(PhysicsDemoMenuView&& other) noexcept = delete;

    void update(const vax::engine::FrameTime& frameTime) override;

  private:

    void _showSimpleDemo();
};
} // namespace vax::ui