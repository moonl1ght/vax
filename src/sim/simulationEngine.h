#pragma once

#include "logger.h"
#include "physicsEngine.h"
#include "simulationConfig.h"
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

    bool setup(const SimulationConfig& config) noexcept;

    void run() noexcept;

  private:
    dev::Logger _logger = dev::Logger("SimulationEngine");

    std::unique_ptr<core::Thread> _thread;

    std::unique_ptr<physics::PhysicsEngine> _physicsEngine;
};

} // namespace vax::sim