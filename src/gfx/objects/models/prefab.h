#pragma once

#include "components.h"
#include "transform.h"
#include <optional>
#include <string>
#include <vector>

namespace vax::gfx {
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
} // namespace vax::gfx