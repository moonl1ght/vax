#pragma once

#include "physicsEngine.h"
#include "scene.h"
#include "world.h"

namespace vax::engine {
template <typename DerivedController> class SceneController {
  public:
    SceneController(std::unique_ptr<Scene> scene, std::shared_ptr<vax::ecs::World> world)
        : _scene(std::move(scene))
        , _world(std::move(world))
        , _physicsEngine(std::make_unique<vax::physics::PhysicsEngine>()) {}

    virtual ~SceneController() = default;

    SceneController(const SceneController& other) = delete;
    SceneController& operator=(const SceneController& other) = delete;
    SceneController(SceneController&& other) noexcept = delete;
    SceneController& operator=(SceneController&& other) noexcept = delete;

    Scene& getScene() { return *_scene; }

    void update(vax::engine::SceneUpdateContext sceneUpdateContext) {
        if (_isPhysicsEnabled) {
            _physicsEngine->step(sceneUpdateContext.frameTime);
        }
        _scene->update(sceneUpdateContext);
        static_cast<DerivedController*>(this)->onUpdate(sceneUpdateContext);
    }

    void load() {
      _physicsEngine->loadWorld(_world);
    }

  protected:
    std::unique_ptr<Scene> _scene;
    std::shared_ptr<vax::ecs::World> _world;
    std::unique_ptr<vax::physics::PhysicsEngine> _physicsEngine;

    bool _isPhysicsEnabled = true;
};
} // namespace vax::engine