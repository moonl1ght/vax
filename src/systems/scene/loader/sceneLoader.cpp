#include "sceneLoader.h"

using namespace vax;
using namespace vax::scene;

std::unique_ptr<EditorScene> SceneLoader::loadEditorScene(const std::string& path) {
    auto config = _loadSceneConfig(path);
    if (!config.has_value()) {
        return nullptr;
    }
    return loadEditorScene(config.value());
}

std::unique_ptr<EditorScene> SceneLoader::loadEditorScene(scene::SceneConfig& config) {
    auto loadQueue = _vkEngine.get().queueManager->graphicsQueue;
    auto loadCommandBuffer = _vkEngine.get().commandManager->createSingleTimeCommandBuffer();

    loadCommandBuffer.begin();

    _textureGpuUploader->uploadTextures(loadCommandBuffer);

    loadCommandBuffer.end();
    loadCommandBuffer.submitAndWait(loadQueue);

    _textureGpuUploader->resetAllTexturesStagingBuffers();
    return nullptr;
}

std::optional<scene::SceneConfig> SceneLoader::_loadSceneConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }
    nlohmann::json json;
    file >> json;
    scene::SceneConfig descriptor = json.get<scene::SceneConfig>();
    return descriptor;
}