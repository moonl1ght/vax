#include "physicsEngine.h"
#include <glm/glm.hpp>

using namespace vax::physics;

void PhysicsEngine::loadWorld(std::shared_ptr<vax::ecs::World> world) noexcept {
    _world = std::move(world);
    _loadPhysics();
}

void PhysicsEngine::_loadPhysics() noexcept {
    _dynamicBodies.clear();
    _staticBodies.clear();
    _shapeColliders.clear();
    _contactConstraints.clear();

    _world->eachCoreComponentEntity<ecs::TransformComponent, ecs::RigidBodyComponent>(
        [&](ecs::Entity entity,
            ecs::TransformComponent& transformComponent,
            ecs::RigidBodyComponent& rigidBodyComponent) {
            vax::physics::RigidBody body{
                .position = transformComponent.transform.position,
                .previousPosition = transformComponent.transform.position,
                .velocity = glm::vec3(0.0f),
                .entity = entity,
                .rigidBodyComponent = rigidBodyComponent,
            };
            if (rigidBodyComponent.isStatic()) [[unlikely]] {
                _staticBodies.push_back(body);
            } else [[likely]] {
                if (_world->hasCoreComponent<ecs::StaticRigidBodyComponent>(entity)) [[unlikely]] {
                    _staticBodies.push_back(body);
                } else [[likely]] {
                    _dynamicBodies.push_back(body);
                }
            }
        }
    );

    _world->eachCoreComponentEntity<ecs::TransformComponent, ecs::StaticRigidBodyComponent>(
        [&](ecs::Entity entity,
            ecs::TransformComponent& transformComponent,
            ecs::StaticRigidBodyComponent& staticRigidBodyComponent) {
            vax::physics::RigidBody body{
                .position = transformComponent.transform.position,
                .previousPosition = transformComponent.transform.position,
                .velocity = glm::vec3(0.0f),
                .entity = entity,
                .rigidBodyComponent = {},
            };
            _staticBodies.push_back(body);
        }
    );
}

void PhysicsEngine::step(const engine::FrameTime& frameTime) noexcept {

    auto frameDeltaTime = frameTime.deltaTimePrecise;

    if (frameDeltaTime > MAX_ACCUMULATED_TIME) [[unlikely]] {
        frameDeltaTime = MAX_ACCUMULATED_TIME;
    }

    _accumulatedTime += frameDeltaTime;

    while (_accumulatedTime >= FIXED_STEP) [[likely]] {
        _accumulatedTime -= FIXED_STEP;

        for (int i = 0; i < substeps; ++i) {
            kinematicsSystem.step(_dynamicBodies, sdt);
            collisionDetector.detectCollisions();
            xpbdSolver.solve();
            kinematicsSystem.updateVelocities(_dynamicBodies, sdt);
        }
    }

    float alpha = static_cast<float>(_accumulatedTime.count()) / FIXED_STEP.count();

    worldUpdater.update(*_world, _dynamicBodies);
}