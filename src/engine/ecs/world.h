#pragma once

#include "componentPool.h"
#include "entity.h"
#include "logger.h"
#include <cassert>
#include <cstdint>
#include <tuple>
#include <vector>

namespace vax::ecs {
class World final {
  public:
    using ComponentTypeId = uint32_t;

    template <typename... ComponentTypes> struct Include {};
    template <typename... ComponentTypes> struct Exclude {};

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

    template <typename ComponentType, typename... Args>
    ComponentType& addCoreComponentFor(Entity entity, Args&&... args) {
        assert(isEntityAlive(entity));
        return _findCoreComponentPool<ComponentType>().emplace(entity.index, std::forward<Args>(args)...);
    }

    template <typename ComponentType> void removeCoreComponentFor(Entity entity) {
        if (!isEntityAlive(entity)) {
            return;
        }
        auto& pool = _findCoreComponentPool<ComponentType>();
        pool.remove(entity.index);
    }

    template <typename ComponentType> bool hasCoreComponent(Entity entity) const {
        const auto& pool = _findCoreComponentPool<ComponentType>();
        return isEntityAlive(entity) && pool.contains(entity.index);
    }

    template <typename ComponentType> ComponentType* getCoreComponentFor(Entity entity) {
        if (!isEntityAlive(entity)) {
            return nullptr;
        }
        auto& pool = _findCoreComponentPool<ComponentType>();
        return pool.get(entity.index);
    }

    template <typename ComponentType> const ComponentType* getCoreComponentFor(Entity entity) const {
        if (!isEntityAlive(entity)) {
            return nullptr;
        }
        const auto& pool = _findCoreComponentPool<ComponentType>();
        return pool.get(entity.index);
    }

    template <typename Func> void forEachCoreComponentPool(Func&& func) {
        std::apply(
            [&func](auto&... pool) {
                (..., func(pool));
            },
            _pools
        );
    }

    template <typename... ComponentTypes, typename Function> void eachCoreComponent(Function&& function) {
        eachCoreComponent(Include<ComponentTypes...>{}, Exclude<>{}, std::forward<Function>(function));
    }

    template <typename... ComponentTypes, typename... ExcludedTypes, typename Function>
    void eachCoreComponent(Include<ComponentTypes...>, Exclude<ExcludedTypes...>, Function&& function) {
        static_assert(sizeof...(ComponentTypes) > 0, "eachCoreComponent requires at least one component type");
        auto pools = std::make_tuple(&_findCoreComponentPool<ComponentTypes>()...);
        auto excludedPools = std::make_tuple(&_findCoreComponentPool<ExcludedTypes>()...);
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
            bool hasExcludedComponent = std::apply(
                [&](auto*... pool) { return (pool->contains(index) || ...); }, excludedPools
            );
            if (hasExcludedComponent) {
                continue;
            }
            Entity entity{.index = index, .generation = _generations[index]};
            std::apply([&](auto*... pool) { function(entity, *pool->get(index)...); }, pools);
        }
    }

  private:
    vax::Logger _logger = vax::Logger("ECS_World");

    std::vector<uint32_t> _generations;
    std::vector<uint32_t> _freeIndices;

    CoreComponentPools _pools;

    template <typename ComponentType> ComponentPool<ComponentType>& _findCoreComponentPool() {
        return std::get<ComponentPool<ComponentType>>(_pools);
    }

    template <typename ComponentType> const ComponentPool<ComponentType>& _findCoreComponentPool() const {
        return std::get<ComponentPool<ComponentType>>(_pools);
    }
};
} // namespace vax::ecs
