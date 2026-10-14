#pragma once

#include "scene.h"
#include "world.h"

namespace vax::engine {
template <typename DerivedController> class SceneController {
  public:
    SceneController(std::unique_ptr<Scene> scene, std::shared_ptr<vax::ecs::World> world)
        : _scene(std::move(scene))
        , _world(std::move(world)) {}

    virtual ~SceneController() = default;

    SceneController(const SceneController& other) = delete;
    SceneController& operator=(const SceneController& other) = delete;
    SceneController(SceneController&& other) noexcept = delete;
    SceneController& operator=(SceneController&& other) noexcept = delete;

    Scene& getScene() { return *_scene; }

    void update(vax::engine::SceneUpdateContext sceneUpdateContext) {
        _scene->update(sceneUpdateContext);
        static_cast<DerivedController*>(this)->onUpdate(sceneUpdateContext);
    }

  protected:
    std::unique_ptr<Scene> _scene;
    std::shared_ptr<vax::ecs::World> _world;
};
} // namespace vax::engine