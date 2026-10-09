#pragma once

#include "debugView.h"
#include "viewBuilder.h"

namespace vax::ui {
class PhysicsDemoMenuView final : public DebugView {
  public:
    enum class Action {
        GO_TO_MAIN_MENU = 0,
        SHOW_SIMPLE_DEMO = 1,
    };

    PhysicsDemoMenuView(vax::engine::Renderer& renderer)
        : DebugView(renderer) {}

    ~PhysicsDemoMenuView() override = default;

    PhysicsDemoMenuView(const PhysicsDemoMenuView& other) = delete;
    PhysicsDemoMenuView& operator=(const PhysicsDemoMenuView& other) = delete;
    PhysicsDemoMenuView(PhysicsDemoMenuView&& other) noexcept = delete;
    PhysicsDemoMenuView& operator=(PhysicsDemoMenuView&& other) noexcept = delete;

    void update(const vax::engine::FrameTime& frameTime) override;

  private:
    std::optional<Action> _pendingAction;

    std::optional<Action> _popPendingAction() { return std::exchange(_pendingAction, std::nullopt); }

    void _handleAction(Action action);

    void _showSimpleDemo();
};
} // namespace vax::ui