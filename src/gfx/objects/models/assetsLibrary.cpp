#include "assetsLibrary.h"

using namespace vax::gfx;
using namespace vax::rhi;
using namespace vax;

void AssetsLibrary::preload(const std::vector<scene::PrefabConfig>& modelConfigs) {
    for (const auto& modelConfig : modelConfigs) {
        if (_prefabs.contains(modelConfig.id)) {
            continue;
        }
        auto prefabResult = _prefabLoader->loadPrefab(modelConfig);
        if (!prefabResult.has_value()) {
            _logger.error("Failed to load prefab: ", modelConfig.id);
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
        prefab.id = modelConfig.id;
        _prefabs.emplace(modelConfig.id, std::move(prefab));
    }
}