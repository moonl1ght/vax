#pragma once

#include "physicsComponents.h"

namespace vax::physics {

class XPBDSolver final {
  public:
    constexpr static float EPSILON = 1e-6f;

    XPBDSolver() = default;
    ~XPBDSolver() = default;

    void solve(BodyStorage& bodyStorage, std::vector<ContactConstraint>& contacts, float dt);

    void solveVelocities(
        BodyStorage& bodyStorage, const std::vector<ContactConstraint>& contacts, const glm::vec3& gravity, float dt
    );
};

} // namespace vax::physics