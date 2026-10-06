#pragma once

#include "componentPool.h"
#include "entity.h"
#include "logger.h"
#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>

namespace vax::ecs {
using ComponentTypeId = uint32_t;

inline ComponentTypeId nextComponentTypeId() {
    static ComponentTypeId nextId = 0;
    return nextId++;
}

template <typename T> ComponentTypeId componentTypeId() {
    static const ComponentTypeId id = nextComponentTypeId();
    return id;
}

class World final {
  public:
    World() = default;
    ~World() = default;

    World(const World& other) = delete;
    World& operator=(const World& other) = delete;
    World(World&& other) noexcept = delete;
    World& operator=(World&& other) noexcept = delete;

    Entity createEntity();

    void destroyEntity(Entity entity);

    bool isEntityAlive(Entity entity) const;

    size_t aliveEntityCount() const { return _generations.size() - _freeIndices.size(); }

    template <typename ComponentType, typename... Args> ComponentType& addComponentFor(Entity entity, Args&&... args) {
        assert(isEntityAlive(entity));
        return _getOrCreatePool<ComponentType>().emplace(entity.index, std::forward<Args>(args)...);
    }

    template <typename ComponentType> void removeComponentFor(Entity entity) {
        if (!isEntityAlive(entity)) {
            return;
        }
        if (auto* pool = _findPool<ComponentType>()) {
            pool->remove(entity.index);
        }
    }

    template <typename ComponentType> bool hasComponent(Entity entity) const {
        const auto* pool = _findPool<ComponentType>();
        return pool != nullptr && isEntityAlive(entity) && pool->contains(entity.index);
    }

    template <typename ComponentType> std::optional<ComponentType> getComponentFor(Entity entity) {
        if (!isEntityAlive(entity)) {
            return std::nullopt;
        }
        auto* pool = _findPool<ComponentType>();
        return pool != nullptr ? std::optional<ComponentType>(pool->get(entity.index)) : std::nullopt;
    }

    template <typename ComponentType> std::optional<const ComponentType> getComponentFor(Entity entity) const {
        if (!isEntityAlive(entity)) {
            return std::nullopt;
        }
        const auto* pool = _findPool<ComponentType>();
        return pool != nullptr ? std::optional<const ComponentType>(pool->get(entity.index)) : std::nullopt;
    }

    template <typename... ComponentTypes, typename Function> void each(Function&& function) {
        static_assert(sizeof...(ComponentTypes) > 0, "each requires at least one component type");
        auto pools = std::make_tuple(_findPool<ComponentTypes>()...);
        bool hasMissingPool = std::apply([](auto*... pool) { return ((pool == nullptr) || ...); }, pools);
        if (hasMissingPool) {
            return;
        }
        const IComponentPool* smallestPool = std::apply(
            [](auto*... pool) {
                const IComponentPool* smallest = nullptr;
                ((smallest = (smallest == nullptr || pool->size() < smallest->size())
                                 ? static_cast<const IComponentPool*>(pool)
                                 : smallest),
                 ...);
                return smallest;
            },
            pools
        );
        for (uint32_t index : smallestPool->entities()) {
            bool hasAllComponents =
                std::apply([index](auto*... pool) { return (pool->contains(index) && ...); }, pools);
            if (!hasAllComponents) {
                continue;
            }
            Entity entity{.index = index, .generation = _generations[index]};
            std::apply([&](auto*... pool) { function(entity, pool->get(index)...); }, pools);
        }
    }

  private:
    vax::Logger _logger = vax::Logger("ECS_World");

    std::vector<uint32_t> _generations;
    std::vector<uint32_t> _freeIndices;

    std::vector<std::unique_ptr<IComponentPool>> _pools;

    template <typename ComponentType> ComponentPool<ComponentType>* _findPool() {
        ComponentTypeId typeId = componentTypeId<ComponentType>();
        return typeId < _pools.size() ? static_cast<ComponentPool<ComponentType>*>(_pools[typeId].get()) : nullptr;
    }

    template <typename ComponentType> const ComponentPool<ComponentType>* _findPool() const {
        ComponentTypeId typeId = componentTypeId<ComponentType>();
        return typeId < _pools.size() ? static_cast<const ComponentPool<ComponentType>*>(_pools[typeId].get())
                                      : nullptr;
    }

    template <typename ComponentType> ComponentPool<ComponentType>& _getOrCreatePool() {
        ComponentTypeId typeId = componentTypeId<ComponentType>();
        if (typeId >= _pools.size()) {
            _pools.resize(typeId + 1);
        }
        if (!_pools[typeId]) {
            _pools[typeId] = std::make_unique<ComponentPool<ComponentType>>();
        }
        return static_cast<ComponentPool<ComponentType>&>(*_pools[typeId]);
    }
};
} // namespace vax::ecs
