#pragma once

#include "jsonOptional.h"
#include "prefabConfig.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace vax::scene {
struct EntityConfig {
    enum class Type {
        Environment,
        Agent,
    };

    std::string id;
    Type type;
    std::optional<PrefabConfig> prefabConfig;
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    EntityConfig::Type,
    {
    {EntityConfig::Type::Environment, "environment"},
    {EntityConfig::Type::Agent, "agent"},
    }
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EntityConfig, id, type, prefabConfig)
} // namespace vax::scene
