#pragma once

#include "camera.h"
#include "entity.h"
#include "world.h"

namespace vax::scene {

class Gizmo final {
  public:
    explicit Gizmo() noexcept {};
    ~Gizmo() = default;

    Gizmo(const Gizmo&) = delete;
    Gizmo(Gizmo&&) noexcept = delete;
    Gizmo& operator=(const Gizmo&) = delete;
    Gizmo& operator=(Gizmo&&) noexcept = delete;

    void setCamera(const gfx::Camera& camera) { _camera = camera; }

    void setEntity(const ecs::Entity& entity) { _entity = entity; }

    const ecs::World& world() const noexcept { return _world; }

    ecs::World& world() noexcept { return _world; }

  private:
    ecs::Entity _entity;
    gfx::Camera _camera;
    ecs::World _world = ecs::World();
};

} // namespace vax::scene