#pragma once

#include "assetsLibrary.h"
#include "editorScene.h"
#include "logger.h"
#include "pipelineManager.h"
#include "resourceManager.h"
#include "sceneConfig.h"
#include "simulationScene.h"
#include "textureGpuUploader.h"
#include "thread.h"
#include "threadRunner.h"
#include "vkEngine.h"
#include <atomic>
#include <memory>

namespace vax::scene {

class SceneLoader final {
  public:
    static constexpr uint32_t MAX_DRAWABLE_INSTANCES = vax::rhi::MAX_DRAWABLE_INSTANCES;

    struct LoadingResult final {
        std::unique_ptr<scene::EditorScene> editorScene;
        std::unique_ptr<scene::SimulationScene> simulationScene;
        std::shared_ptr<gfx::AssetsLibrary> assetsLibrary;
    };

    using LoadingCallback = std::function<void(std::optional<LoadingResult>)>;

    SceneLoader(
        vax::rhi::VulkanEngine& vkEngine,
        vax::rhi::PipelineManager& pipelineManager,
        vax::rhi::ResourceManager& resourceManager,
        core::ThreadRunner& mainThreadRunner
    )
        : _vkEngine(vkEngine)
        , _pipelineManager(pipelineManager)
        , _resourceManager(resourceManager)
        , _mainThreadRunner(mainThreadRunner) {
        _textureGpuUploader = std::make_unique<vax::rhi::TextureGpuUploader>();
    }

    ~SceneLoader() { cancel(); }

    SceneLoader(const SceneLoader& other) = delete;
    SceneLoader& operator=(const SceneLoader& other) = delete;
    SceneLoader(SceneLoader&& other) noexcept = delete;
    SceneLoader& operator=(SceneLoader&& other) noexcept = delete;

    void asyncLoadScene(const std::string& path, LoadingCallback callback);

    void asyncLoadScene(const scene::SceneConfig& config, LoadingCallback callback);

    void cancel() noexcept { _isCancelRequested.store(true); }

  private:
    dev::Logger _logger = dev::Logger("SceneLoader");

    std::reference_wrapper<vax::rhi::VulkanEngine> _vkEngine;
    std::reference_wrapper<vax::rhi::PipelineManager> _pipelineManager;
    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;
    std::reference_wrapper<core::ThreadRunner> _mainThreadRunner;

    std::unique_ptr<vax::rhi::TextureGpuUploader> _textureGpuUploader;

    std::atomic<bool> _isCancelRequested = false;

    core::Thread _loadThread;

    std::optional<scene::SceneConfig> _loadSceneConfig(const std::string& path);

    void _loadInBackground(const scene::SceneConfig& config, LoadingCallback callback);

    std::shared_ptr<gfx::AssetsLibrary> _loadAssetsLibrary(const scene::SceneConfig& config);

    std::unique_ptr<SimulationScene>
    _buildSimulationScene(const scene::SceneConfig& config, const gfx::AssetsLibrary& assetsLibrary);

    std::unique_ptr<EditorScene> _buildEditorScene(const scene::SceneConfig& config);

    void _uploadPendingTextures();

    void _deliver(LoadingCallback callback, std::optional<LoadingResult> result);
};

} // namespace vax::scene