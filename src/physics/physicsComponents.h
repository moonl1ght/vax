#pragma once

#include "entity.h"
#include <glm/glm.hpp>
#include <variant>

namespace vax::ecs {

struct RigidBodyComponent final {
    float invMass; // inverse mass, 1/mass
    float restitution;
    float friction;
};

struct SphereColliderComponent final {
    float radius;
};

struct PlaneColliderComponent final {
    glm::vec3 normal;
    float distance;
};

} // namespace vax::ecs

namespace vax::physics {

struct ShapeCollider final {
    std::variant<ecs::SphereColliderComponent, ecs::PlaneColliderComponent> shapeColliderDescription;
    uint32_t bodyIndex;
};

struct RigidBody final {
    glm::vec3 position;
    glm::vec3 previousPosition;
    glm::vec3 velocity;

    ecs::RigidBodyComponent rigidBodyDescription;
};

struct ContactConstraint final {
    ecs::Entity entityA;
    ecs::Entity entityB;
    glm::vec3 point;
    glm::vec3 normal;
    float penetrationDepth;
    float compliance = 0.0f;
    float lagrangeMultiplier = 0.0f;
};

} // namespace vax::physics