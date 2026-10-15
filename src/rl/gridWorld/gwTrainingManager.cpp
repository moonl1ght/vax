#include "gwTrainingManager.h"
#include "fileSystem.h"
#include "timeManager.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include "tensorOp.h"

using namespace vax;
using namespace vax::rl;
using namespace vax::core;

GWTrainingManager::GWTrainingManager() {
    _trainDirectory = RELATIVE_PATH("output/qlearning/train_" + vax::TimeManager::getCurrentDatetimeString());
    std::filesystem::create_directories(_trainDirectory);
    _fsLogger = std::make_shared<vax::FsLogger>(_trainDirectory + "/training.log");
    _logger.setFsLogger(_fsLogger);
    _logger.setMode(vax::Logger::Mode::FILE);
}

void GWTrainingManager::setInititialGrid(const vax::math::Tensor& grid) {
    _initialGrid = std::make_optional(vax::math::TensorOp::copy(grid));
}

void GWTrainingManager::startTraining(ThreadRunner& threadRunner, std::function<TrainingCallback> callback) {
    std::lock_guard<std::mutex> lock(_trainingMutex);
    if (_isTraining)
        return;

    _isTraining = true;

    _trainingThread = std::jthread([this, &threadRunner, callback = std::move(callback)](std::stop_token st) mutable {
        _setupTraining(threadRunner, callback);
        _train(threadRunner, callback);
    });
}

void GWTrainingManager::_setupTraining(ThreadRunner& threadRunner, std::function<TrainingCallback>& callback) {
    _trainingEngine = std::make_unique<vax::rl::TrainingEngine>();
    _gridWorld = std::make_unique<vax::rl::GridWorld>(_qlConfig);
    _gridWorld->setFsLogger(_fsLogger);
    _gridWorld->setEvalModeImpl(vax::rl::EvalMode::TRAINING);
    _gridWorld->createRandomGrid();
    if (_initialGrid.has_value()) {
        _gridWorld->setInitialGrid(std::move(_initialGrid.value()));
        _initialGrid = std::nullopt;
    }
    _trainingEngine->setFsLogger(_fsLogger);
}

void GWTrainingManager::_train(ThreadRunner& threadRunner, std::function<TrainingCallback>& callback) {
    threadRunner.runOnThread([callback]() { callback(TrainingStatus{"Training in progress...", false}); });
    _trainingEngine->train<GridWorld, GWAgent, State, MoveAction>(
        *_gridWorld, _gridWorld->getAgent(), _qlConfig.episodes
    );
    {
        std::lock_guard<std::mutex> lock(_trainingMutex);
        _isTraining = false;
    }
    _saveTrainingData();
    threadRunner.runOnThread([callback]() { callback(TrainingStatus{"Training completed", true}); });
}

void GWTrainingManager::_saveTrainingData() {
    auto path = _trainDirectory + "/config.json";
    nlohmann::json config;
    config["learningRate"] = _qlConfig.learningRate;
    config["gamma"] = _qlConfig.gamma;
    config["epsilon"] = _qlConfig.epsilon;
    config["episodes"] = _qlConfig.episodes;
    std::ofstream outFile(path);
    outFile << config.dump(4);
    outFile.close();

    _gridWorld->save(_trainDirectory);
}