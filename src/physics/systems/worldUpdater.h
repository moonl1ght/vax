#pragma once

#include "physicsComponents.h"
#include "world.h"
#include <vector>

namespace vax::physics {

class WorldUpdater final {
  public:
    WorldUpdater() = default;
    ~WorldUpdater() = default;

    void update(vax::ecs::World& world, const std::vector<RigidBody>& dynamicBodies) noexcept;
};

} // namespace vax::physics