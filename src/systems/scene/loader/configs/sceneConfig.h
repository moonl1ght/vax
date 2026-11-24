#pragma once

#include "entityConfig.h"
#include "gizmoConfig.h"
#include "jsonOptional.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <string>

namespace vax::scene {

struct SceneConfig final {
    enum class SceneType { GridWorld, Other };

    std::string name;
    bool shouldDrawGizmo;
    SceneType sceneType;
    std::optional<GizmoConfig> gizmoConfig;
    std::vector<EntityConfig> entities;

    auto forEachEntityOfType(EntityConfig::Type type) const {
        return entities | std::views::filter([type](const EntityConfig& entity) { return entity.type == type; });
    }
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    SceneConfig::SceneType,
    {
    {SceneConfig::SceneType::GridWorld, "gridWorld"},
    {SceneConfig::SceneType::Other, "other"},
    }
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    SceneConfig, name, shouldDrawGizmo, sceneType, gizmoConfig, entities
)

} // namespace vax::scene