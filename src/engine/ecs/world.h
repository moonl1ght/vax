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

    template <typename T, typename... Args> T& addComponentFor(Entity entity, Args&&... args) {
        assert(isEntityAlive(entity));
        return _getOrCreatePool<T>().emplace(entity.index, std::forward<Args>(args)...);
    }

    template <typename T> void removeComponentFor(Entity entity) {
        if (!isEntityAlive(entity)) {
            return;
        }
        if (auto* pool = _findPool<T>()) {
            pool->remove(entity.index);
        }
    }

    template <typename T> bool hasComponent(Entity entity) const {
        const auto* pool = _findPool<T>();
        return pool != nullptr && isEntityAlive(entity) && pool->contains(entity.index);
    }

    template <typename T> std::optional<T> getComponentFor(Entity entity) {
        if (!isEntityAlive(entity)) {
            return std::nullopt;
        }
        auto* pool = _findPool<T>();
        return pool != nullptr ? std::optional<T>(pool->get(entity.index)) : std::nullopt;
    }

    template <typename T> std::optional<const T> getComponentFor(Entity entity) const {
        if (!isEntityAlive(entity)) {
            return std::nullopt;
        }
        const auto* pool = _findPool<T>();
        return pool != nullptr ? std::optional<const T>(pool->get(entity.index)) : std::nullopt;
    }

    template <typename... Ts, typename F> void each(F&& f) {
        static_assert(sizeof...(Ts) > 0, "each requires at least one component type");
        auto pools = std::make_tuple(_findPool<Ts>()...);
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
            std::apply([&](auto*... pool) { f(entity, pool->get(index)...); }, pools);
        }
    }

  private:
    vax::Logger _logger = vax::Logger("ECS_World");

    std::vector<uint32_t> _generations;
    std::vector<uint32_t> _freeIndices;

    std::vector<std::unique_ptr<IComponentPool>> _pools;

    template <typename T> ComponentPool<T>* _findPool() {
        ComponentTypeId typeId = componentTypeId<T>();
        return typeId < _pools.size() ? static_cast<ComponentPool<T>*>(_pools[typeId].get()) : nullptr;
    }

    template <typename T> const ComponentPool<T>* _findPool() const {
        ComponentTypeId typeId = componentTypeId<T>();
        return typeId < _pools.size() ? static_cast<const ComponentPool<T>*>(_pools[typeId].get()) : nullptr;
    }

    template <typename T> ComponentPool<T>& _getOrCreatePool() {
        ComponentTypeId typeId = componentTypeId<T>();
        if (typeId >= _pools.size()) {
            _pools.resize(typeId + 1);
        }
        if (!_pools[typeId]) {
            _pools[typeId] = std::make_unique<ComponentPool<T>>();
        }
        return static_cast<ComponentPool<T>&>(*_pools[typeId]);
    }
};
} // namespace vax::ecs
