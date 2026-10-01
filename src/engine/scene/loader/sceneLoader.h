#pragma once

#include "drawableScene.h"

namespace vax::engine {

class SceneLoader {
  public:
    SceneLoader(vax::vk::Engine& vkEngine) : _vkEngine(vkEngine) {}
    ~SceneLoader() = default;

    SceneLoader(const SceneLoader& other) = delete;
    SceneLoader& operator=(const SceneLoader& other) = delete;
    SceneLoader(SceneLoader&& other) noexcept = default;
    SceneLoader& operator=(SceneLoader&& other) noexcept = default;

    std::unique_ptr<DrawableScene> load(const std::string& path);

  private:
    std::reference_wrapper<vax::vk::Engine> _vkEngine;
    std::string _name;
};

} // namespace vax::engine