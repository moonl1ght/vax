#pragma once

#include "physicsComponents.h"
#include "world.h"

namespace vax::physics {

class WorldUpdater final {
  public:
    WorldUpdater() = default;
    ~WorldUpdater() = default;

    void update(vax::ecs::World& world, const BodyStorage& bodyStorage) noexcept;
};

} // namespace vax::physics