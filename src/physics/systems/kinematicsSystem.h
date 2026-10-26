#pragma once

#include "physicsComponents.h"
#include <vector>

namespace vax::physics {

class KinematicsSystem final {
  public:
    constexpr static float EARTH_GRAVITY = -9.81f;

    KinematicsSystem() = default;
    ~KinematicsSystem() = default;

    void step(std::vector<vax::physics::RigidBody>& dynamicBodies, float deltaTime) noexcept;

    void updateVelocities(std::vector<RigidBody>& bodies, float deltaTime) noexcept;

    void setGravity(glm::vec3 gravity) noexcept;

  private:
    glm::vec3 _gravity = glm::vec3(0.0f, EARTH_GRAVITY, 0.0f);
};

} // namespace vax::physics