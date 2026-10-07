#pragma once

#include "entity.h"
#include "transform.h"
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <limits>
#include <string>

namespace vax::ecs {
struct LocalTransformComponent final {
    vax::math::Transform transform;
};

struct WorldTransformComponent final {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 normalMatrix = glm::mat4(1.0f);
};

struct TransformDirtyComponent final {};

struct HierarchyComponent final {
    Entity parent = NullEntity;
    Entity firstChild = NullEntity;
    Entity nextSibling = NullEntity;
    Entity prevSibling = NullEntity;
    uint32_t depth = 0;
};

using DrawableModelId = uint32_t;
inline constexpr DrawableModelId NullDrawableModelId = std::numeric_limits<DrawableModelId>::max();

struct DrawableModelComponent final {
    DrawableModelId model = NullDrawableModelId;
};

struct NameComponent final {
    std::string value;
};

struct HighlightComponent final {
    uint32_t packedColor = 0;
};

struct InstanceComponent final {
    uint32_t typeIndex = 0;
    uint32_t instanceIndex = 0;
};

struct AgentComponent final {};

struct BackgroundComponent final {};

struct GizmoComponent final {};
} // namespace vax::ecs
