#include "sceneComposer.h"
#include "profiler.h"
#include "shaderUniforms.h"
#include "transform.h"
#include <algorithm>
#include <numeric>

using namespace vax::engine;
using namespace vax::ecs;

void SceneComposer::updateTransforms(World& world) {
    ZoneScopedN("SceneComposer::updateTransforms");
    _transformEntries.clear();
    world.each<LocalTransformComponent, WorldTransformComponent, HierarchyComponent, NameComponent>(
        [&](Entity entity,
            LocalTransformComponent& local,
            WorldTransformComponent& worldTransform,
            HierarchyComponent& hierarchy,
            NameComponent& name) {
            DebugEntity debugEntity{entity, name.value};
            std::optional<NameComponent> parentName = world.getComponentFor<NameComponent>(hierarchy.parent);
            DebugEntity parentEntity{hierarchy.parent, parentName ? parentName->value : ""};
            _transformEntries.push_back({
                .depth = hierarchy.depth,
                .entity = debugEntity,
                .parent = parentEntity,  
                .local = &local,
                .world = &worldTransform,
            });
        }
    );
    std::stable_sort(_transformEntries.begin(), _transformEntries.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.depth < rhs.depth;
    });

    _entryIndexByEntity.assign(_entryIndexByEntity.size(), -1);
    _isEntryUpdated.assign(_transformEntries.size(), 0);
    for (size_t i = 0; i < _transformEntries.size(); ++i) {
        uint32_t index = _transformEntries[i].entity.entity.index;
        if (index >= _entryIndexByEntity.size()) {
            _entryIndexByEntity.resize(index + 1, -1);
        }
        _entryIndexByEntity[index] = static_cast<int32_t>(i);
    }

    for (size_t i = 0; i < _transformEntries.size(); ++i) {
        auto& entry = _transformEntries[i];
        int32_t parentEntryIndex = -1;
        if (!entry.parent.entity.isNull() && entry.parent.entity.index < _entryIndexByEntity.size()) {
            parentEntryIndex = _entryIndexByEntity[entry.parent.entity.index];
        }
        bool isParentUpdated = parentEntryIndex >= 0 && _isEntryUpdated[parentEntryIndex];
        if (!isParentUpdated && !world.hasComponent<TransformDirtyComponent>(entry.entity.entity)) {
            continue;
        }
        glm::mat4 parentMatrix =
            parentEntryIndex >= 0 ? _transformEntries[parentEntryIndex].world->modelMatrix : glm::mat4(1.0f);
        vax::math::TransformMatrixHandle matrixHandle(parentMatrix * entry.local->transform.getModelMatrix());
        entry.world->modelMatrix = matrixHandle.getModelMatrix();
        entry.world->normalMatrix = matrixHandle.getNormalMatrix();
        _isEntryUpdated[i] = 1;
    }

    _dirtyEntities.clear();
    world.each<TransformDirtyComponent>([&](Entity entity, TransformDirtyComponent&) {
        _dirtyEntities.push_back(entity);
    });
    for (Entity entity : _dirtyEntities) {
        world.removeComponentFor<TransformDirtyComponent>(entity);
    }
}

uint32_t SceneComposer::compose(
    World& world, IndirectDrawController& indirectDrawController, uint32_t frameIndex, uint32_t baseInstance
) {
    ZoneScopedN("SceneComposer::compose");
    const size_t modelCount = _assetsLibrary->drawableModelCount();

    _modelInstanceCounts.assign(modelCount, 0);
    world.each<WorldTransformComponent, DrawableModelComponent>(
        [&](Entity, WorldTransformComponent&, DrawableModelComponent& drawableModel) {
            if (drawableModel.model < modelCount) {
                ++_modelInstanceCounts[drawableModel.model];
            }
        }
    );

    const uint32_t totalInstances =
        std::accumulate(_modelInstanceCounts.begin(), _modelInstanceCounts.end(), baseInstance);
    if (totalInstances > _maxInstances) {
        _logger.error("Instance buffer overflow: ", totalInstances, " > ", _maxInstances);
        return baseInstance;
    }

    _modelInstanceOffsets.resize(modelCount);
    std::exclusive_scan(
        _modelInstanceCounts.begin(), _modelInstanceCounts.end(), _modelInstanceOffsets.begin(), baseInstance
    );
    _modelInstanceCursors = _modelInstanceOffsets;

    world.each<WorldTransformComponent, DrawableModelComponent>(
        [&](Entity entity, WorldTransformComponent& worldTransform, DrawableModelComponent& drawableModel) {
            if (drawableModel.model >= modelCount) {
                return;
            }
            InstanceData instanceData{
                .model = worldTransform.modelMatrix,
                .normalMatrix = worldTransform.normalMatrix,
                .packedColor = 0,
                .flags = InstanceFlags::InstanceFlagsNone,
                .instanceId = entity.index,
                .padding = 0,
            };
            if (world.hasComponent<HighlightComponent>(entity)) {
                instanceData.packedColor = world.getComponentFor<HighlightComponent>(entity)->packedColor;
                instanceData.flags = InstanceFlags::IsInstanceSelected;
            }
            _ssboManager.get().updateInstance(frameIndex, _modelInstanceCursors[drawableModel.model]++, instanceData);
        }
    );

    for (uint32_t modelId = 0; modelId < modelCount; ++modelId) {
        if (_modelInstanceCounts[modelId] == 0) {
            continue;
        }
        _assetsLibrary->drawableModel(modelId).prepareDrawing(
            &indirectDrawController,
            frameIndex,
            DrawableModel::DrawSettings{
            .instanceOffset = _modelInstanceOffsets[modelId],
            .instancesCount = _modelInstanceCounts[modelId],
            }
        );
    }
    return totalInstances;
}