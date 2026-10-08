#pragma once

#include "agent.h"
#include "gwenv.h"
#include "prefabDescriptor.h"
#include "logger.h"
#include "luna.h"
#include "qlConfig.h"
#include "vaxMath.h"
#include "tensor.h"

namespace vax::rl {
class GridWorld;
}

namespace vax::rl {
class GWAgent final : public vax::rl::Agent<GWAgent, State, MoveAction> {
  public:
    explicit GWAgent(vax::rl::QLearningConfig qlConfig)
        : _qlConfig(qlConfig) {};

    GWAgent(
        vax::rl::QLearningConfig qlConfig,
        const vax::math::Position2DInt& startPosition,
        vax::math::Tensor&& qTable
    )
        : _qlConfig(qlConfig)
        , _startPosition(startPosition)
        , _qTable(std::move(qTable)) {};

    ~GWAgent() = default;

    GWAgent(const GWAgent& other) = delete;
    GWAgent& operator=(const GWAgent& other) = delete;
    GWAgent(GWAgent&& other) noexcept = delete;
    GWAgent& operator=(GWAgent&& other) noexcept = delete;

    vax::engine::PrefabDescriptor getPrefabDescriptor() const;

    void moveByOutsideAction(MoveAction action);

    void allowAction(MoveAction action);

    void linkGridWorld(vax::rl::GridWorld* gridWorld);

    const vax::math::Position2DInt& getPosition() const;
    const vax::math::Position2DInt& getOldPosition() const;

    void setStartPosition(int x, int y) {
        _startPosition = {x, y};
        _position = _startPosition;
    }

    vax::math::Position2DInt getNewPosition(MoveAction action) const;

    MoveAction chooseActionImpl(const State& state);

    void updateImpl(const State& state, MoveAction action, double reward, const State& nextState, bool done);

    void reset();

    void setEvalModeImpl(vax::rl::EvalMode evalMode);

    const vax::math::Tensor& getQTable() const { return _qTable; }

    const vax::math::Position2DInt& getStartPosition() const { return _startPosition; }

    void setQTable(vax::math::Tensor&& qTable);

    void setQLearningConfig(const vax::rl::QLearningConfig& qlConfig);

    void setFsLogger(std::shared_ptr<vax::FsLogger> fsLogger);

    AgentOrientation getOrientation() const { return _orientation; }

    void toggleAllowRandomActionsDuringEval() { _allowRandomActionsDuringEval = !_allowRandomActionsDuringEval; }

  private:
    vax::Logger _logger = vax::Logger("GWAgent");
    vax::rl::QLearningConfig _qlConfig;
    vax::math::Position2DInt _startPosition = {0, 0};
    vax::math::Position2DInt _position = {0, 0};
    vax::math::Position2DInt _oldPosition = {0, 0};
    vax::rl::GridWorld* _gridWorld = nullptr;
    vax::rl::EvalMode _evalMode = vax::rl::EvalMode::EVALUATION;
    AgentOrientation _orientation = AgentOrientation::NORTH;
    bool _allowRandomActionsDuringEval = false;

    vax::math::Tensor _qTable;

    void _tryToMove(MoveAction action);
    bool _canTakeAction(MoveAction action) const;
    void _updateOrientation(MoveAction action);
};
} // namespace vax::rl::gw