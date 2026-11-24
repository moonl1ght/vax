#include "simulationEngine.h"
#include "physicalCore.h"

using namespace vax::sim;

bool SimulationEngine::setup(
    const SimulationConfig& config, std::unique_ptr<scene::SimulationScene> simulationScene
) noexcept {
    _simulationScene = std::move(simulationScene);
    _thread = std::make_unique<core::Thread>();
    if (config.enableThreadAffinity) {
        auto cores = core::PhysicalCore::cores();
        // TODO: update thread affinity, we should pin all threads in the app to cores.
        // For now, I just pin the simulation thread to the third core and non 0 core as 0 core used by main thread.
        if (cores.size() < 3) {
            _logger.error("Less than 3 physical cores found, will not pin thread to any core");
        } else {
            _thread->setPinnedCore(cores[2]);
            _logger.info("Thread pinned to core: ", cores[2].primary_logical_id);
        }
    }

    if (config.physicsConfig) {
        _physicsEngine = std::make_unique<physics::PhysicsEngine>();
        if (!_physicsEngine->setup(config.physicsConfig.value())) {
            _logger.error("Failed to setup physics engine");
            return false;
        }
    }
    return true;
}

void SimulationEngine::run() noexcept {
    _thread->start([this]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
}