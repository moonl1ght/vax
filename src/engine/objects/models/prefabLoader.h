#pragma once

#include "modelLoader.h"
#include "prefab.h"
#include "resourceManager.h"
#include "modelDescriptor.h"

namespace vax::engine {
class PrefabLoader {
  public:
    PrefabLoader(vax::engine::ModelLoader& modelLoader, vax::vk::ResourceManager& resourceManager)
        : _modelLoader(modelLoader)
        , _resourceManager(resourceManager) {};

    ~PrefabLoader() = default;

    PrefabLoader(const PrefabLoader& other) = delete;
    PrefabLoader& operator=(const PrefabLoader& other) = delete;
    PrefabLoader(PrefabLoader&& other) noexcept = delete;
    PrefabLoader& operator=(PrefabLoader&& other) noexcept = delete;

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>> loadPrefab(const ModelDescriptor& descriptor);

  private:
    vax::Logger _logger = vax::Logger("PrefabLoader");

    std::reference_wrapper<vax::engine::ModelLoader> _modelLoader;
    std::reference_wrapper<vax::vk::ResourceManager> _resourceManager;

    std::optional<std::pair<Prefab, std::vector<DrawableModel>>> _loadURDFPrefab(const ModelDescriptor& descriptor);
};
} // namespace vax::engine