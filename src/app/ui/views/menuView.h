#pragma once

#include "statsView.h"
#include "trainingView.h"
#include "view.h"
#include "viewBuilder.h"

namespace vax::ui {
class MenuView final : public View {
  public:
    enum class Action {
        SHOW_ROVER_DEMO,
        TRAIN_Q_LEARNING,
        SHOW_PHYSICS_ENGINE_DEMO,
    };

    MenuView(ViewBuilder& viewBuilder, vax::engine::Renderer& renderer)
        : View(renderer)
        , _viewBuilder(viewBuilder) {
        _statsView = std::make_unique<StatsView>(renderer);
    };

    ~MenuView() override = default;

    MenuView(const MenuView& other) = delete;
    MenuView& operator=(const MenuView& other) = delete;
    MenuView(MenuView&& other) noexcept = delete;
    MenuView& operator=(MenuView&& other) noexcept = delete;

    void update(const vax::engine::FrameTime& frameTime) override;

  private:
    std::reference_wrapper<ViewBuilder> _viewBuilder;
    std::unique_ptr<TrainingView> _trainingView = nullptr;
    std::unique_ptr<StatsView> _statsView = nullptr;
    std::optional<Action> _pendingAction;
    bool _showTrainingStatus = false;

    std::optional<Action> _popPendingAction() { return std::exchange(_pendingAction, std::nullopt); }

    void _handleAction(Action action);
};
} // namespace vax::ui