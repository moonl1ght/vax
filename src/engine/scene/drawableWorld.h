#pragma once

#include "world.h"

namespace vax::engine {

class DrawableWorld {
  public:
    explicit DrawableWorld(vax::ecs::World& world)
        : _world(world) {}

    ~DrawableWorld() = default;

    DrawableWorld(const DrawableWorld&) = delete;
    DrawableWorld(DrawableWorld&&) = delete;
    DrawableWorld& operator=(const DrawableWorld&) = delete;
    DrawableWorld& operator=(DrawableWorld&&) = delete;

    vax::ecs::World& world() { return _world.get(); }

    const vax::ecs::World& world() const { return _world.get(); }

  protected:
    std::reference_wrapper<vax::ecs::World> _world;
};

} // namespace vax::engine