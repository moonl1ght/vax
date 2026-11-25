#include "sceneLoader.h"
#include <fstream>
#include <nlohmann/json.hpp>

using namespace vax;
using namespace vax::scene;

void SceneLoader::asyncLoadScene(const std::string& path, LoadingCallback callback) {
    _isCancelRequested.store(false);
    _loadThread.start([this, path, callback]() {
        if (_isCancelRequested.load()) {
            _deliver(callback, std::nullopt);
            return;
        }
        auto config = _loadSceneConfig(path);
        if (!config.has_value()) {
            _logger.error("Failed to load scene config: ", path);
            _deliver(callback, std::nullopt);
            return;
        }
        _loadInBackground(config.value(), callback);
    });
}

void SceneLoader::asyncLoadScene(const scene::SceneConfig& config, LoadingCallback callback) {
    _isCancelRequested.store(false);
    _loadThread.start([this, config, callback]() { _loadInBackground(config, callback); });
}

void SceneLoader::_loadInBackground(const scene::SceneConfig& config, LoadingCallback callback) {
    for (int i = 0; i < 10 && !_isCancelRequested.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (_isCancelRequested.load()) {
        _logger.info("Scene loading cancelled");
        _deliver(callback, std::nullopt);
        return;
    }

    auto assetsLibrary = _loadAssetsLibrary(config);

    LoadingResult result;
    result.simulationScene = _buildSimulationScene(config, *assetsLibrary);
    result.editorScene = _buildEditorScene(config);
    result.assetsLibrary = std::move(assetsLibrary);

    _mainThreadRunner.get().runOnThread([this, callback = std::move(callback), result = std::move(result)]() mutable {
        if (_isCancelRequested.load()) {
            _logger.debug("Scene loading cancelled");
            _textureGpuUploader->resetAllTexturesStagingBuffers();
            callback(std::nullopt);
            return;
        }
        _uploadPendingTextures();
        callback(std::move(result));
    });
}

std::shared_ptr<gfx::AssetsLibrary> SceneLoader::_loadAssetsLibrary(const scene::SceneConfig& config) {
    _resourceManager.get().load(MAX_DRAWABLE_INSTANCES);

    auto assetsLibrary = std::make_shared<gfx::AssetsLibrary>(
        *_vkEngine.get().device,
        MAX_DRAWABLE_INSTANCES,
        _resourceManager.get(),
        *_textureGpuUploader,
        _pipelineManager.get().descriptorRegistry()
    );

    std::vector<scene::PrefabConfig> prefabConfigs;
    prefabConfigs.reserve(config.entities.size());
    for (const auto& entity : config.entities) {
        if (entity.prefabConfig.has_value()) {
            prefabConfigs.push_back(*entity.prefabConfig);
        }
    }
    assetsLibrary->preload(prefabConfigs);

    return assetsLibrary;
}

std::unique_ptr<SimulationScene>
SceneLoader::_buildSimulationScene(const scene::SceneConfig& config, const gfx::AssetsLibrary& assetsLibrary) {
    return std::make_unique<SimulationScene>();
}

std::unique_ptr<EditorScene> SceneLoader::_buildEditorScene(const scene::SceneConfig& config) {
    auto gizmo = std::make_unique<scene::Gizmo>();
    return std::make_unique<scene::EditorScene>(std::move(gizmo));
}

void SceneLoader::_uploadPendingTextures() {
    auto& loadQueue = _vkEngine.get().queueManager->getMainGraphicsQueue();
    auto loadCommandBuffer = _vkEngine.get().commandManager->createSingleTimeCommandBuffer();

    loadCommandBuffer.begin();
    _textureGpuUploader->uploadTextures(loadCommandBuffer);
    loadCommandBuffer.end();
    loadCommandBuffer.submitAndWait(loadQueue);

    _textureGpuUploader->resetAllTexturesStagingBuffers();
}

void SceneLoader::_deliver(LoadingCallback callback, std::optional<LoadingResult> result) {
    _mainThreadRunner.get().runOnThread([callback = std::move(callback), result = std::move(result)]() mutable {
        callback(std::move(result));
    });
}

std::optional<scene::SceneConfig> SceneLoader::_loadSceneConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }
    try {
        nlohmann::json json;
        file >> json;
        return json.get<scene::SceneConfig>();
    } catch (const nlohmann::json::exception& e) {
        _logger.error("Failed to parse scene config ", path, ": ", e.what());
        return std::nullopt;
    }
}
