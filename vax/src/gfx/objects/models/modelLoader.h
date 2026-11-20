#pragma once

#include "drawableModel.h"
#include "resourceManager.h"
#include "textureLoader.h"
#include <functional>

namespace vax::gfx {
class ModelLoader final {
  public:
    explicit ModelLoader(const vax::rhi::Device& device, vax::rhi::ResourceManager& resourceManager)
        : _resourceManager(resourceManager) {
        _textureLoader = std::make_unique<vax::rhi::TextureLoader>(device, resourceManager.textureManager());
    };

    ~ModelLoader() {};

    ModelLoader(const ModelLoader& other) = delete;
    ModelLoader& operator=(const ModelLoader& other) = delete;
    ModelLoader(ModelLoader&& other) noexcept = delete;
    ModelLoader& operator=(ModelLoader&& other) noexcept = delete;

    std::optional<vax::gfx::DrawableModel> loadModel(const std::string& path, uint32_t instancesCount = 1);

  private:
    dev::Logger _logger = dev::Logger("ModelLoader");

    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;

    std::unique_ptr<vax::rhi::TextureLoader> _textureLoader;
};
} // namespace vax::gfx