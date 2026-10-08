#pragma once

#include "drawableScene.h"
#include "scene.h"
#include "sceneDescriptor.h"
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

    std::unique_ptr<Scene> load(const std::string& path);

    std::unique_ptr<Scene> load(SceneDescriptor& descriptor);

  private:
    std::reference_wrapper<vax::vk::Engine> _vkEngine;

    std::string _name;

    std::unique_ptr<DrawableScene> _initDrawableScene(ecs::World& world, const SceneDescriptor& descriptor);

    void _loadSceneAndWorld(std::unique_ptr<ecs::World>& world, std::unique_ptr<DrawableScene>& drawableScene);

    std::optional<SceneDescriptor> _loadSceneDescriptor(const std::string& path);
};

} // namespace vax::engine