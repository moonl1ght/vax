#pragma once

#include "drawableScene.h"
#include "world.h"

namespace vax::engine {

class Scene final {
  public:
    Scene(std::unique_ptr<ecs::World> world, std::unique_ptr<DrawableScene> drawableScene)
        : _world(std::move(world))
        , _drawableScene(std::move(drawableScene)) {}

    ~Scene() = default;

    Scene(const Scene& other) = delete;
    Scene& operator=(const Scene& other) = delete;
    Scene(Scene&& other) noexcept = default;
    Scene& operator=(Scene&& other) noexcept = default;

    DrawableScene& drawableScene() { return *_drawableScene; }

    const DrawableScene& drawableScene() const { return *_drawableScene; }

    ecs::World& world() { return *_world; }

    const ecs::World& world() const { return *_world; }

  private:
    std::unique_ptr<ecs::World> _world;
    std::unique_ptr<DrawableScene> _drawableScene;
};

} // namespace vax::engine