#include "sceneLoader.h"
#include "resourceManager.h"

using namespace vax::engine;

std::unique_ptr<DrawableScene> SceneLoader::load(const std::string& path) {
    auto resourceManager = std::make_unique<vax::vk::ResourceManager>(*_vkEngine.get().device);
    auto textureLoader = std::make_unique<vax::vk::TextureLoader>(*_vkEngine.get().device, resourceManager->textureManager(), *_vkEngine.get().commandManager);

    return nullptr;
}