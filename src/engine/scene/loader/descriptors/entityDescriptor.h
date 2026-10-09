#pragma once

#include "jsonOptional.h"
#include "prefabDescriptor.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace vax::engine {
struct EntityDescriptor {
    enum class Type {
        Environment,
        Agent,
    };

    std::string id;
    Type type;
    std::optional<PrefabDescriptor> prefabDescriptor;
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    EntityDescriptor::Type,
    {
    {EntityDescriptor::Type::Environment, "environment"},
    {EntityDescriptor::Type::Agent, "agent"},
    }
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EntityDescriptor, id, type, prefabDescriptor)
} // namespace vax::engine
