#pragma once

#include "gridWorldDescriptor.h"
#include "gwAgent.h"
#include "gwDrawableWorld.h"
#include "gwenv.h"
#include "inputController.h"
#include "logger.h"
#include "qlConfig.h"
#include "rlenv.h"
#include "tensor.h"
#include "vaxMath.h"

namespace vax::rl {
class GridWorld final : public vax::InputController::Observer,
                        public vax::rl::Environment<GridWorld, State, MoveAction> {
  public:
    enum class BlockType : uint8_t {
        FLOOR = 0,
        WALL = 1,
        FINISH = 2,
        START = 3,
        TRAP = 4,
    };

    explicit GridWorld(vax::rl::QLearningConfig qlConfig)
        : _qlConfig(qlConfig) {};

    ~GridWorld() {
        if (_inputController) {
            _inputController->removeObserver(this);
        }
    };

    GridWorld(const GridWorld& other) = delete;
    GridWorld& operator=(const GridWorld& other) = delete;
    GridWorld(GridWorld&& other) noexcept = delete;
    GridWorld& operator=(GridWorld&& other) noexcept = delete;

    void reinitWorld();

    void createRandomGrid();

    void save(const std::string& folderPath);

    bool load(const std::string& folderPath);

    vax::rl::GridWorldDrawableDescriptor getDrawableDescriptor() const;

    bool canMoveAgent(const vax::math::Position2DInt& newPosition) const;

    void onMouseMove(const vax::MouseMoveValue& value) {};

    void onMouseWheel(float delta) {};

    void onKeyEvent(const vax::KeyEvent& keyEvent);

    void linkDrawableWorld(GWDrawableWorld* drawableWorld);

    void agentMoved();

    const vax::math::Tensor& getGrid() const;

    State resetImpl();

    State getStateImpl() const;

    vax::rl::StepResult stepImpl(MoveAction action);

    const std::string& nameImpl() const { return _name; }

    void setEvalModeImpl(vax::rl::EvalMode evalMode);

    vax::rl::GWAgent& getAgent() { return _agent; }

    void setFsLogger(std::shared_ptr<vax::FsLogger> fsLogger);

    void startDemo(std::function<void()> onDone);

    void changeAgentStartPosition();

    void setInitialGrid(vax::math::Tensor&& grid);

  private:
    vax::Logger _logger = vax::Logger("GridWorld");
    vax::rl::QLearningConfig _qlConfig;
    std::string _name = "GridWorld";
    vax::math::Tensor _grid;
    vax::rl::GWAgent _agent = vax::rl::GWAgent(_qlConfig);
    std::vector<vax::math::Position2DFloat> _drawableWorldPositions;
    float _moveSpeed = 1.0f;
    float _rotationSpeed = 0.5f;

    std::string blockTypeToPath(BlockType blockType) const;
    GWDrawableWorld* _drawableWorld;
    vax::rl::EvalMode _evalMode = vax::rl::EvalMode::EVALUATION;

    void _updateAgentPosition();
    void _updateGridInstancesHighlight();
};
} // namespace vax::rl