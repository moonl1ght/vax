#pragma once

#include "uiEngine.h"
#include "view.h"
#include "viewBuilder.h"

namespace vax::ui {
class PhysicsDemoMenuView final : public View {
  public:
    enum class Action {
        GO_TO_MAIN_MENU = 0,
        SHOW_SIMPLE_DEMO = 1,
    };

    PhysicsDemoMenuView(ViewBuilder& viewBuilder, UIEngine& uiEngine, vax::engine::Renderer& renderer)
        : View(renderer)
        , _viewBuilder(viewBuilder)
        , _uiEngine(uiEngine) {}
    ~PhysicsDemoMenuView() override = default;

    PhysicsDemoMenuView(const PhysicsDemoMenuView& other) = delete;
    PhysicsDemoMenuView& operator=(const PhysicsDemoMenuView& other) = delete;
    PhysicsDemoMenuView(PhysicsDemoMenuView&& other) noexcept = delete;
    PhysicsDemoMenuView& operator=(PhysicsDemoMenuView&& other) noexcept = delete;

    void update(const vax::engine::FrameTime& frameTime) override;

  private:
    std::reference_wrapper<ViewBuilder> _viewBuilder;
    std::reference_wrapper<UIEngine> _uiEngine;
    std::optional<Action> _pendingAction;

    std::optional<Action> _popPendingAction() { return std::exchange(_pendingAction, std::nullopt); }

    void _handleAction(Action action);
};
} // namespace vax::ui