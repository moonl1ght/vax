#pragma once

#include "prefab.h"
#include "world.h"

namespace vax::engine {

struct PrefabSpawner final {
  public:
    static vax::ecs::Entity spawnPrefab(
        ecs::World& world, const Prefab& prefab, const vax::math::Transform& rootTransform = vax::math::Transform()
    );
};

} // namespace vax::engine