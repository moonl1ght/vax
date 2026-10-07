#pragma once

#include "drawableScene.h"
#include "gridWorldDescriptor.h"
#include "scene.h"
#include "world.h"

namespace vax::engine {

class SceneLoader {
  public:
    SceneLoader(vax::vk::Engine& vkEngine)
        : _vkEngine(vkEngine) {}
    ~SceneLoader() = default;

    SceneLoader(const SceneLoader& other) = delete;
    SceneLoader& operator=(const SceneLoader& other) = delete;
    SceneLoader(SceneLoader&& other) noexcept = default;
    SceneLoader& operator=(SceneLoader&& other) noexcept = default;

    std::unique_ptr<Scene> load(const std::string& path, const vax::rl::GridWorldDrawableDescriptor& descriptor);

  private:
    std::reference_wrapper<vax::vk::Engine> _vkEngine;

    std::string _name;

    std::unique_ptr<DrawableScene> _initDrawableScene(ecs::World& world, const vax::rl::GridWorldDrawableDescriptor& descriptor);

    void _loadSceneAndWorld(
        std::unique_ptr<ecs::World>& world,
        std::unique_ptr<DrawableScene>& drawableScene
    );
};

} // namespace vax::engine