#pragma once

#include "assetsLibrary.h"
#include "indirectDrawController.h"
#include "logger.h"
#include "ssboManager.h"
#include "world.h"
#include "сomponents.h"
#include <cstdint>
#include <vector>

namespace vax::engine {
class SceneComposer final {
  public:
    SceneComposer(vax::engine::AssetsLibrary& assetsLibrary, vax::vk::SSBOManager& ssboManager, uint32_t maxInstances)
        : _assetsLibrary(assetsLibrary)
        , _ssboManager(ssboManager)
        , _maxInstances(maxInstances) {};

    ~SceneComposer() = default;

    SceneComposer(const SceneComposer& other) = delete;
    SceneComposer& operator=(const SceneComposer& other) = delete;
    SceneComposer(SceneComposer&& other) noexcept = delete;
    SceneComposer& operator=(SceneComposer&& other) noexcept = delete;

    void updateTransforms(vax::ecs::World& world);

    void
    compose(vax::ecs::World& world, vax::engine::IndirectDrawController& indirectDrawController, uint32_t frameIndex);

  private:
    struct TransformEntry final {
        uint32_t depth;
        vax::ecs::Entity entity;
        vax::ecs::Entity parent;
        vax::ecs::LocalTransformComponent* local;
        vax::ecs::WorldTransformComponent* world;
    };

    vax::Logger _logger = vax::Logger("SceneComposer");

    std::reference_wrapper<vax::engine::AssetsLibrary> _assetsLibrary;
    std::reference_wrapper<vax::vk::SSBOManager> _ssboManager;
    uint32_t _maxInstances;

    std::vector<TransformEntry> _transformEntries;
    std::vector<int32_t> _entryIndexByEntity;
    std::vector<uint8_t> _isEntryUpdated;
    std::vector<vax::ecs::Entity> _dirtyEntities;

    std::vector<uint32_t> _modelInstanceCounts;
    std::vector<uint32_t> _modelInstanceOffsets;
    std::vector<uint32_t> _modelInstanceCursors;
};
} // namespace vax::engine