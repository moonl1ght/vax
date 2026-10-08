#pragma once

#include "entityDescriptor.h"
#include "gizmoDescriptor.h"
#include "jsonOptional.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <string>

namespace vax::engine {

struct SceneDescriptor final {
    enum class SceneType { GridWorld, Other };

    std::string name;
    SceneType sceneType;
    std::optional<GizmoDescriptor> gizmoDescriptor;
    std::vector<EntityDescriptor> entities;

    auto forEachEntityOfType(EntityDescriptor::Type type) const {
        return entities | std::views::filter([type](const EntityDescriptor& entity) { return entity.type == type; });
    }
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    SceneDescriptor::SceneType,
    {
    {SceneDescriptor::SceneType::GridWorld, "gridWorld"},
    {SceneDescriptor::SceneType::Other, "other"},
    }
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SceneDescriptor, name, sceneType, gizmoDescriptor, entities)

} // namespace vax::engine