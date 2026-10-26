#pragma once

#include "entity.h"
#include <glm/glm.hpp>
#include <variant>

namespace vax::ecs {

struct RigidBodyComponent final {
    float invMass = 0.0f; // inverse mass, 1/mass
    float restitution = 0.0f;
    float friction = 0.0f;

    // Better to set StaticRigidBodyComponent instead of invMass == 0.0f
    bool isStatic() const { return invMass == 0.0f; }
};

// if entity has RigidBodyComponent and StaticRigidBodyComponent, it treats as static, so try to avoid adding both
// components to the same entity
struct StaticRigidBodyComponent final {};

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
    std::variant<ecs::SphereColliderComponent, ecs::PlaneColliderComponent> shapeColliderComponent;
    uint32_t bodyIndex;
};

struct RigidBody final {
    glm::vec3 position;
    glm::vec3 previousPosition;
    glm::vec3 velocity;

    ecs::Entity entity;

    ecs::RigidBodyComponent rigidBodyComponent;
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