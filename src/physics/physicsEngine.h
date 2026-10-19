#pragma once

#include "collisionDetector.h"
#include "frameTime.h"
#include "kinematicsSystem.h"
#include "logger.h"
#include "world.h"
#include "worldUpdater.h"
#include "xpbdSolver.h"
#include <chrono>

namespace vax::physics {

class PhysicsEngine {
  public:
    using Tick60Hz = std::chrono::duration<int64_t, std::ratio<1, 60>>;
    constexpr static auto FIXED_STEP = std::chrono::duration_cast<std::chrono::nanoseconds>(Tick60Hz(1));
    constexpr static auto MAX_ACCUMULATED_TIME = std::chrono::milliseconds(200);

    PhysicsEngine(std::shared_ptr<vax::ecs::World> world) noexcept
        : _world(std::move(world)) {

        }

    ~PhysicsEngine() noexcept = default;

    PhysicsEngine(const PhysicsEngine& other) = delete;
    PhysicsEngine& operator=(const PhysicsEngine& other) = delete;
    PhysicsEngine(PhysicsEngine&& other) noexcept = delete;
    PhysicsEngine& operator=(PhysicsEngine&& other) noexcept = delete;

    void step(const engine::FrameTime& frameTime) noexcept;

  private:
    static constexpr int substeps = 8;
    static constexpr float sdt = 1.0f / substeps;

    vax::Logger _logger = vax::Logger("PhysicsEngine");

    std::shared_ptr<vax::ecs::World> _world;

    std::chrono::nanoseconds _accumulatedTime = std::chrono::nanoseconds::zero();

    KinematicsSystem kinematicsSystem;
    CollisionDetector collisionDetector;
    WorldUpdater worldUpdater;
    XPBDSolver xpbdSolver;

    std::vector<vax::ecs::Entity> _entities;
    std::vector<vax::physics::RigidBody> _rigidBodies;
    std::vector<vax::physics::ShapeCollider> _shapeColliders;

    std::vector<vax::physics::ContactConstraint> _contactConstraints;
};

} // namespace vax::physics