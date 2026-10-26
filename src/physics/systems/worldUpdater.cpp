#include "worldUpdater.h"

using namespace vax::physics;

void WorldUpdater::update(vax::ecs::World& world, const std::vector<vax::physics::RigidBody>& dynamicBodies) noexcept {
    for (auto& body : dynamicBodies) {
        auto transformComponent = world.getCoreComponentFor<ecs::TransformComponent>(body.entity);
        if (transformComponent) [[likely]] {
            transformComponent->transform.position = body.position;
            world.addCoreComponentFor<ecs::TransformDirtyComponent>(body.entity);
        }
    }
}