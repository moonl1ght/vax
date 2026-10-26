#pragma once

#include "logger.h"
#include "scene.h"
#include "sceneDescriptor.h"
#include "world.h"

namespace vax::engine {

class SceneLoader {
  public:
    typedef std::pair<std::unique_ptr<Scene>, std::shared_ptr<ecs::World>> SceneWorldPair;

    SceneLoader(vax::vk::Engine& vkEngine)
        : _vkEngine(vkEngine) {}
    ~SceneLoader() = default;

    SceneLoader(const SceneLoader& other) = delete;
    SceneLoader& operator=(const SceneLoader& other) = delete;
    SceneLoader(SceneLoader&& other) noexcept = default;
    SceneLoader& operator=(SceneLoader&& other) noexcept = default;

    std::optional<SceneWorldPair> load(const std::string& path);

    SceneWorldPair load(SceneDescriptor& descriptor);

  private:
    vax::Logger _logger = vax::Logger("SceneLoader");

    std::reference_wrapper<vax::vk::Engine> _vkEngine;

    std::string _name;

    std::unique_ptr<Scene> _initScene(ecs::World& world, const SceneDescriptor& descriptor);

    std::optional<SceneDescriptor> _loadSceneDescriptor(const std::string& path);
};

} // namespace vax::engine