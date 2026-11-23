#include "kinematicsSystem.h"

using namespace vax::physics;

void KinematicsSystem::setGravity(glm::vec3 gravity) noexcept { _gravity = gravity; }

void KinematicsSystem::step(BodyStorage& bodyStorage, float deltaTime) noexcept {
    for (size_t i = 0; i < bodyStorage.dynamicBodies.size(); ++i) {
        auto& body = bodyStorage.dynamicBodies[i];
        body.velocity += _gravity * deltaTime;

        body.previousPosition = body.position;

        body.position += body.velocity * deltaTime;
    }
}

void KinematicsSystem::updateVelocities(BodyStorage& bodyStorage, float deltaTime) noexcept {
    const float invDeltaTime = 1.0f / deltaTime;
    for (size_t i = 0; i < bodyStorage.dynamicBodies.size(); ++i) {
        auto& body = bodyStorage.dynamicBodies[i];
        body.velocity = (body.position - body.previousPosition) * invDeltaTime;
    }
}