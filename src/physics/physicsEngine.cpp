#include "physicsEngine.h"
#include <glm/glm.hpp>
#include <iostream>
// #include "objects.h"

using namespace vax::physics;

void PhysicsEngine::step(const engine::FrameTime& frameTime) noexcept {

    auto frameDeltaTime = frameTime.deltaTimePrecise;

    if (frameDeltaTime > MAX_ACCUMULATED_TIME) {
        frameDeltaTime = MAX_ACCUMULATED_TIME;
    }

    _accumulatedTime += frameDeltaTime;

    while (_accumulatedTime >= FIXED_STEP) {
        _accumulatedTime -= FIXED_STEP;

        kinematicsSystem.step();
        collisionDetector.detectCollisions();
        worldUpdater.update();
        xpbdSolver.solve();
    }

    float alpha = static_cast<float>(_accumulatedTime.count()) / FIXED_STEP.count();

    // std::cout << "PhysicsEngine test" << std::endl;

    // Particle sphere(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f);

    // const float dt = 1.0f / 60.0f;
    // const int substeps = 10;
    // const float sdt = dt / substeps;

    // const glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
    // const float restitution = 0.5f;

    // const float groundY = 0.0f;
    // float compliance = 0.0f;
    // float lambda = 0.0f;

    // for (int frame = 0; frame < 180; ++frame) {
    //     lambda = 0.0f;
    //     for (int i = 0; i < substeps; ++i) {

    //         glm::vec3 x_old = sphere.position;

    //         sphere.velocity = sphere.velocity + gravity * sdt;
    //         sphere.position = sphere.position + sphere.velocity * sdt;

    //         float constraint = (sphere.position.y - sphere.radius) - groundY;
    //         bool hit = constraint < 0.0f;
    //         float deltaLambda = 0.0f;
    //         if (hit) {
    //             float alphaTilde = compliance / (sdt * sdt);
    //             float w = sphere.invMass;

    //             float deltaLambda = (-constraint - alphaTilde * lambda) / (w + alphaTilde);

    //             float oldLambda = lambda;
    //             lambda = std::max(0.0f, lambda + deltaLambda);
    //             float actualDeltaLambda = lambda - oldLambda;

    //             sphere.position.y += sphere.invMass * actualDeltaLambda;
    //         }

    //         sphere.velocity = (sphere.position - x_old) * (1.0f / sdt);
    //     }
    // }
    // std::cout << "Final position: " << sphere.position.x << ", " << sphere.position.y << ", " << sphere.position.z <<
    // std::endl;
}