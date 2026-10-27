#include "physicsEngine.h"
#include <glm/glm.hpp>

using namespace vax::physics;

void PhysicsEngine::loadWorld(std::shared_ptr<vax::ecs::World> world) noexcept {
    _world = std::move(world);
    _loadPhysics();
}

void PhysicsEngine::_loadPhysics() noexcept {
    _bodyStorage.clear();
    _contactConstraints.clear();

    _world->eachCoreComponentEntity<ecs::TransformComponent, ecs::RigidBodyComponent, ecs::SphereColliderComponent>(
        [&](ecs::Entity entity,
            ecs::TransformComponent& transformComponent,
            ecs::RigidBodyComponent& rigidBodyComponent,
            ecs::SphereColliderComponent& shapeColliderComponent) {
            vax::physics::DynamicBody body{
                .position = transformComponent.transform.position,
                .previousPosition = transformComponent.transform.position,
                .velocity = glm::vec3(0.0f),
                .entity = entity,
                .rigidBodyComponent = rigidBodyComponent,
            };
            if (rigidBodyComponent.isStatic()) [[unlikely]] {
                vax::physics::StaticBody staticBody{
                    .position = transformComponent.transform.position,
                    .orientation = transformComponent.transform.orientation,
                    .entity = entity,
                };
                _bodyStorage.addStaticBody(staticBody, shapeColliderComponent);
            } else [[likely]] {
                if (_world->hasCoreComponent<ecs::StaticRigidBodyComponent>(entity)) [[unlikely]] {
                    vax::physics::StaticBody staticBody{
                        .position = transformComponent.transform.position,
                        .orientation = transformComponent.transform.orientation,
                        .entity = entity,
                    };
                    _bodyStorage.addStaticBody(staticBody, shapeColliderComponent);
                } else [[likely]] {
                    _bodyStorage.addDynamicBody(body, shapeColliderComponent);
                }
            }
        }
    );

    _world
        ->eachCoreComponentEntity<ecs::TransformComponent, ecs::StaticRigidBodyComponent, ecs::SphereColliderComponent>(
            [&](ecs::Entity entity,
                ecs::TransformComponent& transformComponent,
                ecs::StaticRigidBodyComponent& staticRigidBodyComponent,
                ecs::SphereColliderComponent& shapeColliderComponent) {
                vax::physics::StaticBody staticBody{
                    .position = transformComponent.transform.position,
                    .orientation = transformComponent.transform.orientation,
                    .entity = entity,
                };
                _bodyStorage.addStaticBody(staticBody, shapeColliderComponent);
            }
        );

    collisionDetector.buildStaticAABB(_bodyStorage);
}

void PhysicsEngine::step(const engine::FrameTime& frameTime) noexcept {

    auto frameDeltaTime = frameTime.deltaTimePrecise;

    if (frameDeltaTime > MAX_ACCUMULATED_TIME) [[unlikely]] {
        frameDeltaTime = MAX_ACCUMULATED_TIME;
    }

    _accumulatedTime += frameDeltaTime;

    collisionDetector.buildDynamicAABB(_bodyStorage, sdt);
    collisionDetector.findCollisionPairs();

    while (_accumulatedTime >= FIXED_STEP) [[likely]] {
        _accumulatedTime -= FIXED_STEP;

        for (int i = 0; i < substeps; ++i) {
            kinematicsSystem.step(_bodyStorage, sdt);
            collisionDetector.detectCollisions();
            xpbdSolver.solve();
            kinematicsSystem.updateVelocities(_bodyStorage, sdt);
        }
    }

    float alpha = static_cast<float>(_accumulatedTime.count()) / FIXED_STEP.count();

    worldUpdater.update(*_world, _bodyStorage);
}