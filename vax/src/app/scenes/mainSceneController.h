#pragma once

#include "sceneController.h"

namespace vax::app {

class MainSceneController final : public gfx::SceneController<MainSceneController> {
  public:
    MainSceneController(std::unique_ptr<gfx::Scene> scene, std::shared_ptr<ecs::World> world)
        : SceneController(std::move(scene), std::move(world)) {
        _isPhysicsEnabled = true;
    }

    MainSceneController(const MainSceneController& other) = delete;
    MainSceneController& operator=(const MainSceneController& other) = delete;
    MainSceneController(MainSceneController&& other) noexcept = delete;
    MainSceneController& operator=(MainSceneController&& other) noexcept = delete;

    ~MainSceneController() = default;

    void onUpdate(const gfx::SceneUpdateContext& context);
};
} // namespace vax::app