#pragma once

#include "modelLoader.h"
#include "prefab.h"
#include "prefabDescriptor.h"
#include "primitivesBuilder.h"
#include "resourceManager.h"

namespace vax::engine {
class PrefabLoader {
  public:
    PrefabLoader(const vax::vk::Device& device, vax::vk::ResourceManager& resourceManager)
        : _resourceManager(resourceManager) {
        _modelLoader = std::make_unique<vax::engine::ModelLoader>(device, resourceManager);
        _primitivesBuilder = std::make_unique<vax::engine::PrimitivesBuilder>(resourceManager);
    };

    ~PrefabLoader() = default;

    PrefabLoader(const PrefabLoader& other) = delete;
    PrefabLoader& operator=(const PrefabLoader& other) = delete;
    PrefabLoader(PrefabLoader&& other) noexcept = delete;
    PrefabLoader& operator=(PrefabLoader&& other) noexcept = delete;

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>> loadPrefab(const PrefabDescriptor& descriptor);

  private:
    vax::Logger _logger = vax::Logger("PrefabLoader");

    std::reference_wrapper<vax::vk::ResourceManager> _resourceManager;

    std::unique_ptr<vax::engine::ModelLoader> _modelLoader;
    std::unique_ptr<vax::engine::PrimitivesBuilder> _primitivesBuilder;

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>> _loadURDFPrefab(const PrefabDescriptor& descriptor);

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>> _loadGLBPrefab(const PrefabDescriptor& descriptor);

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>>
    _loadPrimitivePrefab(const PrefabDescriptor& descriptor);

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>> _loadAssetPrefab(const PrefabDescriptor& descriptor);
};
} // namespace vax::engine