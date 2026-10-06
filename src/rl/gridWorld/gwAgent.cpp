#include "gwAgent.h"
#include "gridWorld.h"
#include "gwenv.h"
#include "randomGenerator.h"
#include "tensorOp.h"

using namespace vax;
using namespace vax::rl;
using namespace vax::math;
using namespace vax::engine;

vax::engine::PrefabDescriptor GWAgent::getDrawableDescriptor() const {
    auto transform = Transform();
    transform.scale = {0.5f, 0.5f, 0.5f};
    return {
        .path = RES_PATH("assets/models/rover/rover.urdf"),
        .id = "rover",
        .transforms = {transform},
        .selectedInstanceInfos = {},
        .modelType = vax::engine::PrefabDescriptor::ModelType::MODEL,
        .instancesCount = 1,
        .isIdentifiable = true,
    };
}

void GWAgent::allowAction(MoveAction action) {
    _oldPosition = _position;
    _position = getNewPosition(action);
    if (_evalMode == vax::rl::EvalMode::EVALUATION) {
        _updateOrientation(action);
        _gridWorld->agentMoved();
    }
}

void GWAgent::linkGridWorld(vax::rl::GridWorld* gridWorld) {
    _gridWorld = gridWorld;
    _qTable = Tensor::createZeros({_gridWorld->getGrid().totalSize(), numMoveActions});
}

void GWAgent::moveByOutsideAction(MoveAction action) { _tryToMove(action); }

void GWAgent::_tryToMove(MoveAction action) {
    if (_canTakeAction(action)) {
        _oldPosition = _position;
        _position = getNewPosition(action);
        if (_evalMode == vax::rl::EvalMode::EVALUATION) {
            _updateOrientation(action);
            _gridWorld->agentMoved();
        }
    }
}

bool GWAgent::_canTakeAction(MoveAction action) const {
    Position2DInt newPosition = getNewPosition(action);
    if (_gridWorld) {
        return _gridWorld->canMoveAgent(newPosition);
    }
    _logger.warning("Grid world not linked!");
    return false;
}

Position2DInt GWAgent::getNewPosition(MoveAction action) const {
    Position2DInt newPosition = _position;
    switch (action) {
    case MoveAction::NORTH:
        newPosition.y += 1;
        break;
    case MoveAction::SOUTH:
        newPosition.y -= 1;
        break;
    case MoveAction::EAST:
        newPosition.x -= 1;
        break;
    case MoveAction::WEST:
        newPosition.x += 1;
        break;
    }
    return newPosition;
}

const Position2DInt& GWAgent::getPosition() const { return _position; }

const Position2DInt& GWAgent::getOldPosition() const { return _oldPosition; }

MoveAction GWAgent::chooseActionImpl(const State& state) {
    if (_evalMode == vax::rl::EvalMode::TRAINING || _allowRandomActionsDuringEval) {
        core::RandomGenerator& generator = core::RandomGenerator::getInstance();
        if (generator.uniformFloat() < _qlConfig.epsilon) {
            auto action = static_cast<MoveAction>(generator.uniformInt(0, numMoveActions - 1));
            _logger.info("Chosen random action: ", moveActionToString(action));
            return action;
        }
    }
    auto flatIndex = _gridWorld->getGrid().flatIndex({state.x, state.y});
    auto indices = TensorOp::maxOverLastDim(_qTable, {flatIndex});
    if (indices.size() != 2) {
        _logger.error("No action found for state: ", state.x, ", ", state.y);
        return MoveAction::NORTH;
    }
    auto action = static_cast<MoveAction>(indices[1]);
    if (_evalMode == vax::rl::EvalMode::TRAINING) {
        _logger.info("Chosen best action: ", moveActionToString(action));
    }
    return action;
}

void GWAgent::updateImpl(const State& state, MoveAction action, double reward, const State& nextState, bool done) {
    float maxFuture = 0.0f;
    auto flatIndexNextState = _gridWorld->getGrid().flatIndex({nextState.x, nextState.y});
    auto flatIndexState = _gridWorld->getGrid().flatIndex({state.x, state.y});
    if (!done) {
        auto indices = TensorOp::maxOverLastDim(_qTable, {flatIndexNextState});
        if (indices.empty()) {
            _logger.error("No action found for next state: ", nextState.x, ", ", nextState.y);
            return;
        }
        auto maxFutureValue = _qTable.get(indices);
        if (!maxFutureValue.has_value()) {
            _logger.error("No max future value found for next state: ", nextState.x, ", ", nextState.y);
            return;
        }
        maxFuture = maxFutureValue.value();
    }

    float target = reward + _qlConfig.gamma * maxFuture;

    auto prevValue = _qTable.get({flatIndexState, static_cast<int>(action)});
    if (!prevValue.has_value()) {
        _logger.error(
            "No previous value found for state: ", state.x, ", ", state.y, " and action: ", static_cast<int>(action)
        );
        return;
    }
    auto updatedValue = _qlConfig.learningRate * (target - prevValue.value());
    _logger.info(
        "Updated Q value: ",
        updatedValue,
        " current state: ",
        state.x,
        ", ",
        state.y,
        " next state: ",
        nextState.x,
        ", ",
        nextState.y,
        " action: ",
        moveActionToString(action),
        " target: ",
        target,
        " previous value: ",
        prevValue.value(),
        " reward: ",
        reward
    );
    _qTable.set({flatIndexState, static_cast<int>(action)}, updatedValue);
}

void GWAgent::reset() {
    _position = _startPosition;
    _oldPosition = _startPosition;
}

void GWAgent::setEvalModeImpl(vax::rl::EvalMode evalMode) { _evalMode = evalMode; }

void GWAgent::setQTable(vax::math::Tensor&& qTable) { _qTable = std::move(qTable); }

void GWAgent::setQLearningConfig(const vax::rl::QLearningConfig& qlConfig) { _qlConfig = qlConfig; }

void GWAgent::setFsLogger(std::shared_ptr<vax::FsLogger> fsLogger) {
    _logger.setFsLogger(fsLogger);
    _logger.setMode(vax::Logger::Mode::FILE);
}

void GWAgent::_updateOrientation(MoveAction action) {
    switch (action) {
    case MoveAction::NORTH:
        _orientation = AgentOrientation::NORTH;
        break;
    case MoveAction::SOUTH:
        _orientation = AgentOrientation::SOUTH;
        break;
    case MoveAction::EAST:
        _orientation = AgentOrientation::EAST;
        break;
    case MoveAction::WEST:
        _orientation = AgentOrientation::WEST;
        break;
    }
}