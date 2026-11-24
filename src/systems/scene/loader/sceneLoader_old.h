#pragma once

#include "logger.h"
#include "resourceManager.h"
#include "scene.h"
#include "sceneConfig.h"
#include "textureGpuUploader.h"
#include "world.h"

namespace vax::gfx {

class SceneLoader {
  public:
    typedef std::pair<std::unique_ptr<Scene>, std::shared_ptr<ecs::World>> SceneWorldPair;

    SceneLoader(
        vax::rhi::VulkanEngine& vkEngine,
        vax::rhi::PipelineManager& pipelineManager,
        vax::rhi::ResourceManager& resourceManager,
        editor::WindowController& windowController
    )
        : _vkEngine(vkEngine)
        , _pipelineManager(pipelineManager)
        , _resourceManager(resourceManager)
        , _windowController(windowController) {
        _textureGpuUploader = std::make_unique<vax::rhi::TextureGpuUploader>();
    }

    ~SceneLoader() = default;

    SceneLoader(const SceneLoader& other) = delete;
    SceneLoader& operator=(const SceneLoader& other) = delete;
    SceneLoader(SceneLoader&& other) noexcept = default;
    SceneLoader& operator=(SceneLoader&& other) noexcept = default;

    std::optional<SceneWorldPair> load(const std::string& path);

    SceneWorldPair load(scene::SceneConfig& descriptor);

  private:
    dev::Logger _logger = dev::Logger("SceneLoader");

    std::reference_wrapper<vax::rhi::VulkanEngine> _vkEngine;
    std::reference_wrapper<vax::rhi::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;
    std::reference_wrapper<editor::WindowController> _windowController;

    std::unique_ptr<vax::rhi::TextureGpuUploader> _textureGpuUploader;

    std::string _name;

    std::unique_ptr<Scene> _initScene(ecs::World& world, const scene::SceneConfig& descriptor);

    std::optional<scene::SceneConfig> _loadSceneConfig(const std::string& path);
};

} // namespace vax::gfx