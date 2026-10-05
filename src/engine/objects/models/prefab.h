#pragma once

#include "transform.h"
#include "сomponents.h"
#include <optional>
#include <string>
#include <vector>

namespace vax::engine {
struct PrefabNode final {
    std::string name;
    vax::math::Transform localTransform;
    std::optional<vax::ecs::DrawableModelId> model;
    int32_t parentIndex = -1;
};

struct Prefab final {
    std::string id;
    std::vector<PrefabNode> nodes;
};
} // namespace vax::engine