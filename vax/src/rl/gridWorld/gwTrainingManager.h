#pragma once

#include "gridWorld.h"
#include "threadRunner.h"
#include "trainingEngine.h"
#include <thread>
#include "qlConfig.h"

namespace vax::rl {
struct TrainingStatus {
    std::string message = "";
    bool isCompleted = false;
};

using TrainingCallback = void(TrainingStatus trainingStatus);

class GWTrainingManager final {
  public:
    GWTrainingManager();
    ~GWTrainingManager() = default;

    GWTrainingManager(const GWTrainingManager& other) = delete;
    GWTrainingManager& operator=(const GWTrainingManager& other) = delete;
    GWTrainingManager(GWTrainingManager&& other) noexcept = delete;
    GWTrainingManager& operator=(GWTrainingManager&& other) noexcept = delete;

    void startTraining(vax::core::ThreadRunner& threadRunner, std::function<TrainingCallback> callback);

    void setInititialGrid(const vax::math::Tensor& grid);

  private:
    std::shared_ptr<vax::FsLogger> _fsLogger;
    vax::Logger _logger = vax::Logger("GWTrainingManager");
    std::unique_ptr<vax::rl::TrainingEngine> _trainingEngine;
    std::unique_ptr<vax::rl::GridWorld> _gridWorld;
    vax::rl::QLearningConfig _qlConfig;
    std::string _trainDirectory;
    std::optional<vax::math::Tensor> _initialGrid;

    std::jthread _trainingThread;
    std::mutex _trainingMutex;
    bool _isTraining = false;

    void _setupTraining(vax::core::ThreadRunner& threadRunner, std::function<TrainingCallback>& callback);
    void _train(vax::core::ThreadRunner& threadRunner, std::function<TrainingCallback>& callback);
    void _saveTrainingData();
};
} // namespace vax::rl::gw