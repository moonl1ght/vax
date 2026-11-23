#pragma once

#include "physicsConfig.h"

namespace vax::sim {

struct SimulationConfig final {
    static SimulationConfig defaultConfig() noexcept { return SimulationConfig{}; }

    bool enableThreadAffinity = false;

    std::optional<physics::PhysicsConfig> physicsConfig;
};

} // namespace vax::sim