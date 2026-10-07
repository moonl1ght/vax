#pragma once

#include "gizmoDescriptor.h"
#include "jsonOptional.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace vax::engine {

struct SceneDescriptor final {
    enum class SceneType { GridWorld, Other };

    std::string name;
    SceneType sceneType;
    std::optional<GizmoDescriptor> gizmoDescriptor;
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    SceneDescriptor::SceneType,
    {
    {SceneDescriptor::SceneType::GridWorld, "gridWorld"},
    {SceneDescriptor::SceneType::Other, "other"},
    }
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SceneDescriptor, name, sceneType, gizmoDescriptor)

} // namespace vax::engine