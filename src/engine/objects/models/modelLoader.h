#pragma once

#include "drawableModel.h"
#include "resourceManager.h"
#include "textureLoader.h"
#include <functional>

namespace vax::engine {
class AssetsLibrary;
} // namespace vax::engine

namespace vax::engine {
class ModelLoader final {
  public:
    explicit ModelLoader(const vax::vk::Device& device, vax::vk::ResourceManager& resourceManager)
        : _resourceManager(resourceManager) {
        _textureLoader = std::make_unique<vax::vk::TextureLoader>(device, resourceManager.textureManager());
    };

    ~ModelLoader() {};

    ModelLoader(const ModelLoader& other) = delete;
    ModelLoader& operator=(const ModelLoader& other) = delete;
    ModelLoader(ModelLoader&& other) noexcept = delete;
    ModelLoader& operator=(ModelLoader&& other) noexcept = delete;

    std::optional<vax::engine::DrawableModel> loadModel(const std::string& path, uint32_t instancesCount = 1);

  private:
    vax::Logger _logger = vax::Logger("ModelLoader");

    std::reference_wrapper<vax::vk::ResourceManager> _resourceManager;

    std::unique_ptr<vax::vk::TextureLoader> _textureLoader;
};
} // namespace vax::engine