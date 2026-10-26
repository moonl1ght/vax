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

    PhysicsEngine() noexcept = default;

    ~PhysicsEngine() noexcept = default;

    PhysicsEngine(const PhysicsEngine& other) = delete;
    PhysicsEngine& operator=(const PhysicsEngine& other) = delete;
    PhysicsEngine(PhysicsEngine&& other) noexcept = delete;
    PhysicsEngine& operator=(PhysicsEngine&& other) noexcept = delete;

    void step(const engine::FrameTime& frameTime) noexcept;

    void loadWorld(std::shared_ptr<vax::ecs::World> world) noexcept;

  private:
    static constexpr float FIXED_DT = std::chrono::duration<float>(FIXED_STEP).count();
    static constexpr int substeps = 8;
    static constexpr float sdt = FIXED_DT / substeps;

    vax::Logger _logger = vax::Logger("PhysicsEngine");

    std::shared_ptr<vax::ecs::World> _world = nullptr;

    std::chrono::nanoseconds _accumulatedTime = std::chrono::nanoseconds::zero();

    KinematicsSystem kinematicsSystem;
    CollisionDetector collisionDetector;
    WorldUpdater worldUpdater;
    XPBDSolver xpbdSolver;

    std::vector<vax::ecs::Entity> _entities;
    std::vector<vax::physics::RigidBody> _dynamicBodies;
    std::vector<vax::physics::RigidBody> _staticBodies;
    std::vector<vax::physics::ShapeCollider> _shapeColliders;

    std::vector<vax::physics::ContactConstraint> _contactConstraints;

    void _loadPhysics() noexcept;
};

} // namespace vax::physics