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

  protected:
    std::reference_wrapper<vax::ecs::World> _world;
};

} // namespace vax::engine