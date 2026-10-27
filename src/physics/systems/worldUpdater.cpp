#include "worldUpdater.h"

using namespace vax::physics;

void WorldUpdater::update(vax::ecs::World& world, const BodyStorage& bodyStorage) noexcept {
    for (size_t i = 0; i < bodyStorage.dynamicBodies.size(); ++i) {
        auto& body = bodyStorage.dynamicBodies[i];
        auto transformComponent = world.getCoreComponentFor<ecs::TransformComponent>(body.entity);
        if (transformComponent) [[likely]] {
            transformComponent->transform.position = body.position;
            world.addCoreComponentFor<ecs::TransformDirtyComponent>(body.entity);
        }
    }
}