#pragma once

#include "modelLoader.h"
#include "prefab.h"
#include "prefabDescriptor.h"
#include "primitivesBuilder.h"
#include "resourceManager.h"

namespace vax::gfx {
class PrefabLoader {
  public:
    PrefabLoader(const vax::rhi::Device& device, vax::rhi::ResourceManager& resourceManager)
        : _resourceManager(resourceManager) {
        _modelLoader = std::make_unique<vax::gfx::ModelLoader>(device, resourceManager);
        _textureLoader = std::make_unique<vax::rhi::TextureLoader>(device, resourceManager.textureManager());
        _primitivesBuilder = std::make_unique<vax::gfx::PrimitivesBuilder>(resourceManager);
    };

    ~PrefabLoader() = default;

    PrefabLoader(const PrefabLoader& other) = delete;
    PrefabLoader& operator=(const PrefabLoader& other) = delete;
    PrefabLoader(PrefabLoader&& other) noexcept = delete;
    PrefabLoader& operator=(PrefabLoader&& other) noexcept = delete;

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>> loadPrefab(const PrefabDescriptor& descriptor);

  private:
    dev::Logger _logger = dev::Logger("PrefabLoader");

    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;

    std::unique_ptr<vax::gfx::ModelLoader> _modelLoader;
    std::unique_ptr<vax::rhi::TextureLoader> _textureLoader;
    std::unique_ptr<vax::gfx::PrimitivesBuilder> _primitivesBuilder;

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>> _loadURDFPrefab(const PrefabDescriptor& descriptor);

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>> _loadGLBPrefab(const PrefabDescriptor& descriptor);

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>>
    _loadPrimitivePrefab(const PrefabDescriptor& descriptor);

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>> _loadAssetPrefab(const PrefabDescriptor& descriptor);
};
} // namespace vax::gfx