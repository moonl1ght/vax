#pragma once

#include "debugView.h"
#include "viewBuilder.h"

namespace vax::ui {
class MenuView final : public DebugView {
  public:
    MenuView(vax::engine::Renderer& renderer)
        : DebugView(renderer) {};

    ~MenuView() override = default;

    MenuView(const MenuView& other) = delete;
    MenuView& operator=(const MenuView& other) = delete;
    MenuView(MenuView&& other) noexcept = delete;
    MenuView& operator=(MenuView&& other) noexcept = delete;

    void update(const vax::engine::FrameTime& frameTime) override;

  private:
    void _showGridWorldDemo();

    void _showPhysicsEngineDemo();
};
} // namespace vax::ui