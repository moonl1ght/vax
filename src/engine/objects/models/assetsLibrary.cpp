#include "assetsLibrary.h"

using namespace vax::engine;
using namespace vax::vk;
using namespace vax;

void AssetsLibrary::preload(const std::vector<vax::engine::PrefabDescriptor>& modelDescriptors) {
    for (const auto& descriptor : modelDescriptors) {
        if (_prefabs.contains(descriptor.id)) {
            continue;
        }
        auto prefabResult = _prefabLoader->loadPrefab(descriptor);
        if (!prefabResult.has_value()) {
            _logger.error("Failed to load prefab: ", descriptor.id);
            continue;
        }
        auto& [prefab, models] = *prefabResult;
        auto baseModelId = static_cast<vax::ecs::DrawableModelId>(_drawableModels.size());
        for (auto& model : models) {
            _drawableModels.push_back(std::move(model));
        }
        for (auto& node : prefab.nodes) {
            if (node.model.has_value()) {
                *node.model += baseModelId;
            }
        }
        prefab.id = descriptor.id;
        _prefabs.emplace(descriptor.id, std::move(prefab));
    }
}