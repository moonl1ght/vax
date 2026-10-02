#pragma once

#include "drawableScene.h"
#include "gridWorldDescriptor.h"

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

    std::unique_ptr<DrawableScene> load(const std::string& path, const vax::rl::GridWorldDrawableDescriptor& descriptor);

  private:
    std::reference_wrapper<vax::vk::Engine> _vkEngine;

    std::string _name;

    std::unique_ptr<DrawableScene> _initScene();

    void _loadScene(std::unique_ptr<DrawableScene>& scene, const vax::rl::GridWorldDrawableDescriptor& descriptor);
};

} // namespace vax::engine