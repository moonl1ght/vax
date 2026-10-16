#pragma once

#include "entity.h"
#include <glm/glm.hpp>

namespace vax::ecs {

struct ParticleComponent final {
    glm::vec3 position;
    glm::vec3 velocity;
    float radius;
    float invMass; // inverse mass, 1/mass
};

struct RigidBodyComponent final {};

struct SphereColliderComponent final {
    float radius;
};

struct PlaneColliderComponent final {
    glm::vec3 normal;
    float distance;
};

} // namespace vax::ecs

namespace vax::physics {

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