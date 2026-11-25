#pragma once

#include "assetsLibrary.h"
#include "logger.h"
#include "physicsEngine.h"
#include "simulationConfig.h"
#include "simulationScene.h"
#include "thread.h"
#include <memory>

namespace vax::sim {

class SimulationEngine {
  public:
    enum class State {
        RUNNING,
        PAUSED,
    };

    explicit SimulationEngine() noexcept {};

    ~SimulationEngine() = default;

    SimulationEngine(const SimulationEngine& other) = delete;
    SimulationEngine& operator=(const SimulationEngine& other) = delete;
    SimulationEngine(SimulationEngine&& other) noexcept = delete;
    SimulationEngine& operator=(SimulationEngine&& other) noexcept = delete;

    bool setup(
        const sim::SimulationConfig& config,
        std::shared_ptr<gfx::AssetsLibrary> assetsLibrary,
        std::unique_ptr<scene::SimulationScene> simulationScene
    ) noexcept;

    void run() noexcept;

  private:
    dev::Logger _logger = dev::Logger("SimulationEngine");

    std::unique_ptr<core::Thread> _thread;

    std::shared_ptr<gfx::AssetsLibrary> _assetsLibrary;

    std::unique_ptr<physics::PhysicsEngine> _physicsEngine;

    std::unique_ptr<scene::SimulationScene> _simulationScene;
};

} // namespace vax::sim