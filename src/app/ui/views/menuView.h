#pragma once

#include "trainingView.h"
#include "debugView.h"
#include "viewBuilder.h"

namespace vax::ui {
class MenuView final : public DebugView {
  public:
    enum class Action {
        SHOW_GRID_WORLD_DEMO,
        TRAIN_Q_LEARNING,
        SHOW_PHYSICS_ENGINE_DEMO,
    };

    MenuView(vax::engine::Renderer& renderer)
        : DebugView(renderer) {};

    ~MenuView() override = default;

    MenuView(const MenuView& other) = delete;
    MenuView& operator=(const MenuView& other) = delete;
    MenuView(MenuView&& other) noexcept = delete;
    MenuView& operator=(MenuView&& other) noexcept = delete;

    void update(const vax::engine::FrameTime& frameTime) override;

  private:
    std::unique_ptr<TrainingView> _trainingView = nullptr;
    std::optional<Action> _pendingAction;
    bool _showTrainingStatus = false;

    std::optional<Action> _popPendingAction() { return std::exchange(_pendingAction, std::nullopt); }

    void _handleAction(Action action);
};
} // namespace vax::ui