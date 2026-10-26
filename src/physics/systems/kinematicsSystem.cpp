#include "kinematicsSystem.h"

using namespace vax::physics;

void KinematicsSystem::setGravity(glm::vec3 gravity) noexcept { _gravity = gravity; }

void KinematicsSystem::step(std::vector<vax::physics::RigidBody>& dynamicBodies, float deltaTime) noexcept {
    for (auto& body : dynamicBodies) {
        body.velocity += _gravity * deltaTime;

        body.previousPosition = body.position;

        body.position += body.velocity * deltaTime;
    }
}

void KinematicsSystem::updateVelocities(std::vector<RigidBody>& bodies, float h) noexcept {
    const float invDeltaTime = 1.0f / h;
    for (auto& body : bodies) {
        body.velocity = (body.position - body.previousPosition) * invDeltaTime;
    }
}