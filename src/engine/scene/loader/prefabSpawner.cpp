#include "prefabSpawner.h"

using namespace vax::ecs;
using namespace vax::engine;
using namespace vax;

Entity
PrefabSpawner::spawnPrefab(vax::ecs::World& world, const Prefab& prefab, const vax::math::Transform& rootTransform) {
    const size_t count = prefab.nodes.size();
    if (count == 0) {
        return NullEntity;
    }

    std::vector<Entity> entities(count);
    for (size_t i = 0; i < count; ++i) {
        entities[i] = world.createEntity();
    }

    std::vector<HierarchyComponent> hierarchies(count);
    std::vector<int32_t> firstChildIndex(count, -1);

    for (size_t i = 0; i < count; ++i) {
        int32_t parent = prefab.nodes[i].parentIndex;
        if (parent < 0) {
            continue;
        }
        hierarchies[i].parent = entities[parent];
        hierarchies[i].depth = hierarchies[parent].depth + 1;
        if (int32_t next = firstChildIndex[parent]; next >= 0) {
            hierarchies[i].nextSibling = entities[next];
            hierarchies[next].prevSibling = entities[i];
        }
        firstChildIndex[parent] = static_cast<int32_t>(i);
        hierarchies[parent].firstChild = entities[i];
    }

    for (size_t i = 0; i < count; ++i) {
        const auto& node = prefab.nodes[i];
        Entity entity = entities[i];
        world.addComponentFor<NameComponent>(entity, node.name);
        world.addComponentFor<LocalTransformComponent>(
            entity, node.parentIndex < 0 ? rootTransform : node.localTransform
        );
        world.addComponentFor<WorldTransformComponent>(entity);
        world.addComponentFor<TransformDirtyComponent>(entity);
        world.addComponentFor<HierarchyComponent>(entity, hierarchies[i]);
        if (node.model.has_value()) {
            world.addComponentFor<DrawableModelComponent>(entity, *node.model);
        }
    }
    return entities[0];
}