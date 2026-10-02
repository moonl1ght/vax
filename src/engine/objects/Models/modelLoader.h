#pragma once

#include "commandBuffer.h"
#include "drawableModel.h"
#include "drawableNode.h"
#include "luna.h"
#include "modelDescriptor.h"
#include "resourceManager.h"
#include "textureLoader.h"

namespace vax::engine {
class ModelsController;
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

    std::optional<DrawableNode>
    loadSceneModel(vax::engine::ModelsController& modelsController, const vax::engine::ModelDescriptor& descriptor);

    std::optional<vax::engine::DrawableModel> loadModel(const std::string& path, uint32_t instancesCount = 1);

  private:
    vax::Logger _logger = vax::Logger("ModelLoader");

    std::reference_wrapper<vax::vk::ResourceManager> _resourceManager;

    std::unique_ptr<vax::vk::TextureLoader> _textureLoader;

    std::optional<DrawableNode>
    _loadURDFSceneModel(vax::engine::ModelsController& modelsController, vax::engine::ModelDescriptor descriptor);

    std::optional<DrawableNode>
    _loadGLBSceneModel(vax::engine::ModelsController& modelsController, vax::engine::ModelDescriptor descriptor);
};
} // namespace vax::engine