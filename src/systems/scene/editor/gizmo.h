#pragma once

#include "camera.h"
#include "entity.h"

namespace vax::scene {

class Gizmo final {
  public:
    explicit Gizmo() noexcept {};
    ~Gizmo() = default;

    Gizmo(const Gizmo&) = delete;
    Gizmo(Gizmo&&) noexcept = delete;
    Gizmo& operator=(const Gizmo&) = delete;
    Gizmo& operator=(Gizmo&&) noexcept = delete;

  private:
    ecs::Entity _entity;
    gfx::Camera _camera;
};

} // namespace vax::scene