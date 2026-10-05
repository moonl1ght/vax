#include "world.h"

using namespace vax::ecs;

Entity World::createEntity() {
    if (!_freeIndices.empty()) {
        uint32_t index = _freeIndices.back();
        _freeIndices.pop_back();
        return Entity{.index = index, .generation = _generations[index]};
    }
    uint32_t index = static_cast<uint32_t>(_generations.size());
    _generations.push_back(0);
    return Entity{.index = index, .generation = 0};
}

void World::destroyEntity(Entity entity) {
    if (!isEntityAlive(entity)) {
        _logger.warning("destroy called on dead entity, index=", entity.index, " generation=", entity.generation);
        return;
    }
    for (auto& pool : _pools) {
        if (pool) {
            pool->remove(entity.index);
        }
    }
    ++_generations[entity.index];
    _freeIndices.push_back(entity.index);
}

bool World::isEntityAlive(Entity entity) const {
    return !entity.isNull() && entity.index < _generations.size() && _generations[entity.index] == entity.generation;
}
