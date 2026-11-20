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

    auto addDynamicBody = [&](ecs::Entity entity,
                              ecs::TransformComponent& transformComponent,
                              ecs::RigidBodyComponent& rigidBodyComponent,
                              const ShapeCollider& shapeCollider) {
        if (_world->hasCoreComponent<ecs::StaticRigidBodyComponent>(entity)) [[unlikely]] {
            return;
        }
        if (rigidBodyComponent.isStatic()) [[unlikely]] {
            vax::physics::StaticBody staticBody{
                .position = transformComponent.transform.position,
                .orientation = transformComponent.transform.orientation,
                .restitution = rigidBodyComponent.restitution,
                .staticFriction = rigidBodyComponent.staticFriction,
                .dynamicFriction = rigidBodyComponent.dynamicFriction,
                .entity = entity,
            };
            _bodyStorage.addStaticBody(staticBody, shapeCollider);
            return;
        }
        vax::physics::DynamicBody body{
            .position = transformComponent.transform.position,
            .previousPosition = transformComponent.transform.position,
            .velocity = glm::vec3(0.0f),
            .orientation = transformComponent.transform.orientation,
            .entity = entity,
            .rigidBodyComponent = rigidBodyComponent,
        };
        _bodyStorage.addDynamicBody(body, shapeCollider);
    };

    _world->eachCoreComponentEntity<ecs::TransformComponent, ecs::RigidBodyComponent, ecs::SphereColliderComponent>(
        [&](ecs::Entity entity,
            ecs::TransformComponent& transformComponent,
            ecs::RigidBodyComponent& rigidBodyComponent,
            ecs::SphereColliderComponent& shapeColliderComponent) {
            addDynamicBody(entity, transformComponent, rigidBodyComponent, shapeColliderComponent);
        }
    );

    _world->eachCoreComponentEntity<ecs::TransformComponent, ecs::RigidBodyComponent, ecs::PlaneColliderComponent>(
        [&](ecs::Entity entity,
            ecs::TransformComponent& transformComponent,
            ecs::RigidBodyComponent& rigidBodyComponent,
            ecs::PlaneColliderComponent& shapeColliderComponent) {
            addDynamicBody(entity, transformComponent, rigidBodyComponent, shapeColliderComponent);
        }
    );

    auto addStaticBody = [&](ecs::Entity entity,
                             ecs::TransformComponent& transformComponent,
                             ecs::StaticRigidBodyComponent& staticRigidBodyComponent,
                             const ShapeCollider& shapeCollider) {
        vax::physics::StaticBody staticBody{
            .position = transformComponent.transform.position,
            .orientation = transformComponent.transform.orientation,
            .restitution = staticRigidBodyComponent.restitution,
            .staticFriction = staticRigidBodyComponent.staticFriction,
            .dynamicFriction = staticRigidBodyComponent.dynamicFriction,
            .entity = entity,
        };
        _bodyStorage.addStaticBody(staticBody, shapeCollider);
    };

    _world
        ->eachCoreComponentEntity<ecs::TransformComponent, ecs::StaticRigidBodyComponent, ecs::SphereColliderComponent>(
            [&](ecs::Entity entity,
                ecs::TransformComponent& transformComponent,
                ecs::StaticRigidBodyComponent& staticRigidBodyComponent,
                ecs::SphereColliderComponent& shapeColliderComponent) {
                addStaticBody(entity, transformComponent, staticRigidBodyComponent, shapeColliderComponent);
            }
        );

    _world
        ->eachCoreComponentEntity<ecs::TransformComponent, ecs::StaticRigidBodyComponent, ecs::PlaneColliderComponent>(
            [&](ecs::Entity entity,
                ecs::TransformComponent& transformComponent,
                ecs::StaticRigidBodyComponent& staticRigidBodyComponent,
                ecs::PlaneColliderComponent& shapeColliderComponent) {
                addStaticBody(entity, transformComponent, staticRigidBodyComponent, shapeColliderComponent);
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

    while (_accumulatedTime >= FIXED_STEP) [[likely]] {
        _accumulatedTime -= FIXED_STEP;

        collisionDetector.buildDynamicAABB(_bodyStorage, FIXED_DT);
        collisionDetector.findCollisionPairs(_bodyStorage);

        for (int i = 0; i < substeps; ++i) {
            kinematicsSystem.step(_bodyStorage, sdt);
            collisionDetector.detectCollisions(_bodyStorage);
            auto& contacts = collisionDetector.getContactConstraints();
            xpbdSolver.solve(_bodyStorage, contacts, sdt);
            kinematicsSystem.updateVelocities(_bodyStorage, sdt);
            xpbdSolver.solveVelocities(_bodyStorage, contacts, kinematicsSystem.getGravity(), sdt);
        }
    }

    float alpha = static_cast<float>(_accumulatedTime.count()) / FIXED_STEP.count();

    worldUpdater.update(*_world, _bodyStorage);
}