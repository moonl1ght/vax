#pragma once

#include "editorScene.h"
#include "logger.h"
#include "pipelineManager.h"
#include "resourceManager.h"
#include "sceneConfig.h"
#include "textureGpuUploader.h"
#include "vkEngine.h"
#include "world.h"

namespace vax::scene {

class SceneLoader {
  public:
    SceneLoader(
        vax::rhi::VulkanEngine& vkEngine,
        vax::rhi::PipelineManager& pipelineManager,
        vax::rhi::ResourceManager& resourceManager
    )
        : _vkEngine(vkEngine)
        , _pipelineManager(pipelineManager)
        , _resourceManager(resourceManager) {
        _textureGpuUploader = std::make_unique<vax::rhi::TextureGpuUploader>();
    }

    ~SceneLoader() = default;

    SceneLoader(const SceneLoader& other) = delete;
    SceneLoader& operator=(const SceneLoader& other) = delete;
    SceneLoader(SceneLoader&& other) noexcept = default;
    SceneLoader& operator=(SceneLoader&& other) noexcept = default;

    std::unique_ptr<EditorScene> loadEditorScene(const std::string& path);

    std::unique_ptr<EditorScene> loadEditorScene(scene::SceneConfig& config);

  private:
    dev::Logger _logger = dev::Logger("SceneLoader");

    std::reference_wrapper<vax::rhi::VulkanEngine> _vkEngine;
    std::reference_wrapper<vax::rhi::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;

    std::unique_ptr<vax::rhi::TextureGpuUploader> _textureGpuUploader;

    std::optional<scene::SceneConfig> _loadSceneConfig(const std::string& path);
};

} // namespace vax::scene