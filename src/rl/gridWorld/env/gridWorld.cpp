#include "gridWorld.h"
#include "colorPalette.h"
#include "fileSystem.h"
#include "inputController.h"
#include "nlohmann/json.hpp"
#include "randomGenerator.h"
#include "tensorOp.h"
#include "transform.h"

using namespace vax;
using namespace vax::math;
using namespace vax::rl;
using namespace vax::core;

glm::vec4 getBlockColor(GridWorld::BlockType blockType) {
    switch (blockType) {
    case GridWorld::BlockType::FINISH:
        return engine::ColorPalette::Green;
    case GridWorld::BlockType::START:
        return engine::ColorPalette::Blue;
    default:
        return glm::vec4(0.0f);
    }
}

void GridWorld::reinitWorld() {
    createRandomGrid();
    _updateAgentPosition();
    _updateGridInstancesHighlight();
}

void GridWorld::setInitialGrid(vax::math::Tensor&& grid) {
    _grid = std::move(grid);
    for (int i = 0; i < _grid.totalSize(); ++i) {
        std::vector<int> indices = _grid.indices(i);
        auto blockValue = _grid.get(indices);
        if (!blockValue.has_value()) {
            _logger.error("Block value is not set");
            continue;
        }
        auto blockType = static_cast<BlockType>(blockValue.value());
        if (blockType == BlockType::START) {
            _agent.setStartPosition(indices[0], indices[1]);
        }
    }
}

void GridWorld::createRandomGrid() {
    int gridDimX = 6;
    int gridDimY = 6;
    _grid = Tensor::createZeros({gridDimX, gridDimY});
    _drawableWorldPositions.reserve(_grid.totalSize());
    std::vector<int> emptyIndices;
    emptyIndices.reserve(_grid.totalSize());
    for (int i = 0; i < _grid.totalSize(); ++i) {
        std::vector<int> indices = _grid.indices(i);
        auto padding = 1.0f;
        auto offset = 5.0f / 2.0f - 0.5f;
        Position2DFloat position = {indices[0] * padding - offset * padding, indices[1] * padding - offset * padding};
        _drawableWorldPositions.push_back(position);
        if (isBorderIndex(indices, _grid.shape())) {
            _grid.set(indices, static_cast<float>(BlockType::WALL));
        } else {
            emptyIndices.push_back(i);
        }
    }

    auto indexToChoose = core::RandomGenerator::getInstance().uniformInt(0, emptyIndices.size() - 1);
    auto agentPositionIndex = emptyIndices[indexToChoose];
    auto agentPosition = _grid.indices(agentPositionIndex);
    auto lastEmptyIndex = emptyIndices.back();
    emptyIndices[indexToChoose] = lastEmptyIndex;
    emptyIndices.pop_back();
    _agent.setStartPosition(agentPosition[0], agentPosition[1]);
    _agent.linkGridWorld(this);
    _grid.set(agentPosition, static_cast<float>(BlockType::START));

    indexToChoose = core::RandomGenerator::getInstance().uniformInt(0, emptyIndices.size() - 1);
    auto finishPositionIndex = emptyIndices[indexToChoose];
    auto finishPosition = _grid.indices(finishPositionIndex);
    lastEmptyIndex = emptyIndices.back();
    emptyIndices[indexToChoose] = lastEmptyIndex;
    emptyIndices.pop_back();
    _grid.set(finishPosition, static_cast<float>(BlockType::FINISH));
}

void GridWorld::linkDrawableWorld(GWDrawableWorld* drawableWorld) {
    _drawableWorld = drawableWorld;
    _updateAgentPosition();
}

GridWorldDrawableDescriptor GridWorld::getDrawableDescriptor() const {
    GridWorldDrawableDescriptor worldDescriptor;
    worldDescriptor.drawableDescriptors.reserve(_grid.totalSize());
    int flatIndex = 0;
    std::unordered_map<std::string, engine::ModelDescriptor> descriptors;
    for (const auto& block : _grid) {
        BlockType blockType = static_cast<BlockType>(block);
        auto blockTypeString = blockTypeToPath(blockType);
        Transform transform = Transform();
        transform.position = {_drawableWorldPositions[flatIndex].x, 0.0f, _drawableWorldPositions[flatIndex].y};
        if (blockType == BlockType::WALL) {
            transform.position.y = 0.5f;
        }
        if (descriptors.find(blockTypeString) == descriptors.end()) {
            std::vector<engine::ModelDescriptor::SelectedInstanceInfo> selectedInstanceInfos;
            if (blockType == BlockType::START) {
                selectedInstanceInfos.push_back(
                    engine::ModelDescriptor::SelectedInstanceInfo{0, engine::ColorPalette::Blue}
                );
            }
            if (blockType == BlockType::FINISH) {
                selectedInstanceInfos.push_back(
                    engine::ModelDescriptor::SelectedInstanceInfo{0, engine::ColorPalette::Green}
                );
            }
            descriptors[blockTypeString] = engine::ModelDescriptor{
                .primitiveDescriptor = engine::ModelDescriptor::PrimitiveDescriptor(),
                .path = blockTypeString,
                .id = blockTypeString,
                .transforms = {transform},
                .selectedInstanceInfos = selectedInstanceInfos,
                .modelType = engine::ModelDescriptor::ModelType::MODEL,
                .instancesCount = 1,
                .isIdentifiable = true,
            };
        } else {
            auto& descriptor = descriptors[blockTypeString];
            auto instanceIndex = descriptor.instancesCount;
            descriptor.transforms.push_back(transform);
            descriptor.instancesCount += 1;
            descriptor.isIdentifiable = true;
            if (blockType == BlockType::FINISH || blockType == BlockType::START) {
                auto color =
                    (blockType == BlockType::FINISH) ? engine::ColorPalette::Green : engine::ColorPalette::Blue;
                descriptor.selectedInstanceInfos.push_back(
                    engine::ModelDescriptor::SelectedInstanceInfo{instanceIndex, color}
                );
            }
        }
        ++flatIndex;
    }
    for (const auto& [blockType, descriptor] : descriptors) {
        worldDescriptor.drawableDescriptors.push_back(descriptor);
    }
    worldDescriptor.agentDrawableDescriptor = _agent.getDrawableDescriptor();
    return worldDescriptor;
}

std::string GridWorld::blockTypeToPath(BlockType blockType) const {
    switch (blockType) {
    case BlockType::FLOOR:
        return RES_PATH("assets/models/floor.glb");
    case BlockType::WALL:
        return RES_PATH("assets/models/wall.glb");
    case BlockType::TRAP:
        return RES_PATH("assets/models/floor.glb");
    case BlockType::FINISH:
        return RES_PATH("assets/models/floor.glb");
    default:
        return RES_PATH("assets/models/floor.glb");
    }
}

bool GridWorld::canMoveAgent(const Position2DInt& newPosition) const {
    if (newPosition.x < 0 || newPosition.x >= _grid.shape()[0] || newPosition.y < 0 ||
        newPosition.y >= _grid.shape()[1]) {
        return false;
    }
    return _grid.get({newPosition.x, newPosition.y}) != static_cast<float>(BlockType::WALL);
}

void GridWorld::onKeyEvent(const KeyEvent& keyEvent) {
    if (_evalMode == vax::rl::EvalMode::TRAINING) {
        return;
    }
    if (keyEvent.state != KeyEvent::State::DOWN) {
        return;
    }
    MoveAction action;
    switch (keyEvent.key) {
    case vax::KeyCode::A:
        action = MoveAction::WEST;
        break;
    case vax::KeyCode::S:
        action = MoveAction::SOUTH;
        break;
    case vax::KeyCode::D:
        action = MoveAction::EAST;
        break;
    case vax::KeyCode::W:
        action = MoveAction::NORTH;
        break;
    default:
        return;
    }
    if (_drawableWorld->isMovingAgent()) {
        return;
    }
    _agent.moveByOutsideAction(action);
}

void GridWorld::agentMoved() {
    if (_evalMode == vax::rl::EvalMode::TRAINING) {
        return;
    }
    auto newPosition = std::vector<int>({_agent.getPosition().x, _agent.getPosition().y});
    auto sceneGraphPosition = _drawableWorldPositions[_grid.flatIndex(newPosition)];
    _drawableWorld->moveAgentTo(sceneGraphPosition, _agent.getOrientation(), true, _moveSpeed, _rotationSpeed);
}

const Tensor& GridWorld::getGrid() const { return _grid; }

State GridWorld::resetImpl() {
    _agent.reset();
    return _agent.getPosition();
}

StepResult GridWorld::stepImpl(MoveAction action) {
    if (_evalMode == vax::rl::EvalMode::TRAINING) {
        _logger.info("Step: ", moveActionToString(action));
    }
    auto nextPossiblePosition = _agent.getNewPosition(action);
    if (canMoveAgent(nextPossiblePosition)) {
        if (_evalMode == vax::rl::EvalMode::TRAINING) {
            _logger.info("Moving to: ", nextPossiblePosition.x, ", ", nextPossiblePosition.y);
        }
        _agent.allowAction(action);
        auto blockValue = _grid.get({nextPossiblePosition.x, nextPossiblePosition.y});
        if (!blockValue.has_value()) {
            _logger.error("Block value is not set");
            return {.reward = -100.0, .done = true, .finishedWithError = true};
        }
        auto blockType = static_cast<BlockType>(blockValue.value());
        if (blockType == BlockType::TRAP) {
            _logger.info("Reached trap");
            return {.reward = -100.0, .done = true, .finishedWithError = false};
        }
        if (blockType == BlockType::FINISH) {
            _logger.info("Reached finish");
            return {.reward = 100.0, .done = true, .finishedWithError = false};
        }
        return {.reward = -1.0, .done = false, .finishedWithError = false};
    }
    if (_evalMode == vax::rl::EvalMode::TRAINING) {
        _logger.info("Cannot move to: ", nextPossiblePosition.x, ", ", nextPossiblePosition.y);
    }
    return {.reward = -100.0, .done = false, .finishedWithError = false};
}

State GridWorld::getStateImpl() const { return _agent.getPosition(); }

void GridWorld::setEvalModeImpl(vax::rl::EvalMode evalMode) {
    _evalMode = evalMode;
    _agent.setEvalModeImpl(evalMode);
    switch (evalMode) {
    case vax::rl::EvalMode::TRAINING:
        _logger.setMode(vax::Logger::Mode::FILE);
        break;
    case vax::rl::EvalMode::EVALUATION:
        _logger.setMode(vax::Logger::Mode::CONSOLE);
        break;
    }
}

void GridWorld::save(const std::string& folderPath) {
    auto gridPath = folderPath + "/grid.vaxtensor";
    auto qTablePath = folderPath + "/qtable.vaxtensor";
    _grid.saveToBinary(gridPath);
    _agent.getQTable().saveToBinary(qTablePath);
    nlohmann::json info;
    info["agentStartPosition"] = {_agent.getStartPosition().x, _agent.getStartPosition().y};
    std::ofstream outFile(folderPath + "/gridworld_info.json");
    outFile << info.dump(4);
    outFile.close();
}

bool GridWorld::load(const std::string& folderPath) {
    auto gridPath = folderPath + "/grid.vaxtensor";
    auto qTablePath = folderPath + "/qtable.vaxtensor";
    auto grid = Tensor::loadFromBinary(gridPath);
    auto qTable = Tensor::loadFromBinary(qTablePath);
    if (!grid.has_value() && !qTable.has_value()) {
        _logger.error("Failed to load grid and qtable from folder: ", folderPath);
        return false;
    }
    _agent.setQTable(std::move(qTable.value()));

    nlohmann::json config;
    std::ifstream configFile(folderPath + "/config.json");
    configFile >> config;
    configFile.close();
    auto learningRate = config["learningRate"].get<float>();
    auto gamma = config["gamma"].get<float>();
    auto epsilon = config["epsilon"].get<float>();
    auto episodes = config["episodes"].get<int>();
    _qlConfig = vax::rl::QLearningConfig{learningRate, gamma, epsilon, episodes};
    _agent.setQLearningConfig(_qlConfig);

    nlohmann::json info;
    std::ifstream infoFile(folderPath + "/gridworld_info.json");
    infoFile >> info;
    infoFile.close();
    std::array<int, 2> agentStartPosition = info["agentStartPosition"].get<std::array<int, 2>>();
    _agent.setStartPosition(agentStartPosition[0], agentStartPosition[1]);
    _grid = std::move(grid.value());
    _updateAgentPosition();
    _updateGridInstancesHighlight();
    return true;
}

void GridWorld::setFsLogger(std::shared_ptr<vax::FsLogger> fsLogger) {
    _logger.setFsLogger(fsLogger);
    _agent.setFsLogger(fsLogger);
}

void GridWorld::_updateAgentPosition() {
    auto position = _agent.getPosition();
    auto flatIndex = _grid.flatIndex({position.x, position.y});
    auto sceneGraphPosition = _drawableWorldPositions[flatIndex];
    if (_drawableWorld != nullptr) {
        _drawableWorld->moveAgentTo(sceneGraphPosition, _agent.getOrientation(), false);
    }
}

void GridWorld::_updateGridInstancesHighlight() {
    auto blockTypeString = blockTypeToPath(BlockType::FLOOR);
    _drawableWorld->resetInstancesHighlight(blockTypeString);
    uint32_t instanceIndex = 0;
    for (auto& block : _grid) {
        auto blockType = static_cast<BlockType>(block);
        auto blockTypeString = blockTypeToPath(blockType);
        if (blockType == BlockType::FLOOR || blockType == BlockType::START || blockType == BlockType::FINISH) {
            if (blockType == BlockType::START) {
                _drawableWorld->highlightInstance(blockTypeString, instanceIndex, engine::ColorPalette::Blue);
            } else if (blockType == BlockType::FINISH) {
                _drawableWorld->highlightInstance(blockTypeString, instanceIndex, engine::ColorPalette::Green);
            }
            ++instanceIndex;
        }
    }
}

void GridWorld::startDemo(std::function<void()> onDone) {
    _drawableWorld->setOnAllAnimationsCompleted(onDone);
    int iteration = 0;
    while (true) {
        auto state = _agent.getPosition();
        auto action = _agent.chooseAction(state);
        auto stepResult = stepImpl(action);
        if (iteration > 10) {
            _agent.toggleAllowRandomActionsDuringEval();
            _logger.warning("Toggle allow random actions during eval");
            iteration = 0;
        } else {
            ++iteration;
        }
        if (stepResult.done) {
            break;
        }
    }
}

void GridWorld::changeAgentStartPosition() {
    _grid.set({_agent.getStartPosition().x, _agent.getStartPosition().y}, static_cast<float>(BlockType::FLOOR));
    while (true) {
        auto newPositionIndex = core::RandomGenerator::getInstance().uniformInt(0, _grid.totalSize() - 1);
        auto gridIndices = _grid.indices(newPositionIndex);
        auto blockValue = _grid.get(gridIndices);
        if (!blockValue.has_value()) {
            continue;
        }
        auto blockType = static_cast<BlockType>(blockValue.value());
        if (blockType == BlockType::FLOOR || blockType == BlockType::START) {
            auto sceneGraphPosition = _drawableWorldPositions[_grid.flatIndex(gridIndices)];
            _agent.setStartPosition(gridIndices[0], gridIndices[1]);
            _grid.set(gridIndices, static_cast<float>(BlockType::START));
            _drawableWorld->moveAgentTo(sceneGraphPosition, _agent.getOrientation(), false);
            break;
        }
    }
    _updateGridInstancesHighlight();
}