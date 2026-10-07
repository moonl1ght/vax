#include "assetsLibrary.h"
#include "transform.h"
#include <strings.h>

using namespace vax::engine;
using namespace vax::vk;
using namespace vax;

void AssetsLibrary::preloadv2(const std::vector<vax::engine::PrefabDescriptor>& modelDescriptors) {
    for (const auto& descriptor : modelDescriptors) {
        if (_prefabs.contains(descriptor.id)) {
            continue;
        }
        auto prefabResult = _prefabLoader->loadPrefab(descriptor);
        if (!prefabResult.has_value()) {
            _logger.error("Failed to load prefab: ", descriptor.id);
            continue;
        }
        auto& [prefab, models] = *prefabResult;
        auto baseModelId = static_cast<vax::ecs::DrawableModelId>(_drawableModels.size());
        for (auto& model : models) {
            _drawableModels.push_back(std::move(model));
        }
        for (auto& node : prefab.nodes) {
            if (node.model.has_value()) {
                *node.model += baseModelId;
            }
        }
        prefab.id = descriptor.id;
        _prefabs.emplace(descriptor.id, std::move(prefab));
    }
}

std::vector<std::string> AssetsLibrary::getModelIds() const {
    std::vector<std::string> modelIds;
    modelIds.reserve(_modelMap.size());
    for (const auto& [id, _] : _modelMap) {
        modelIds.push_back(id);
    }
    return modelIds;
}

std::optional<DrawableNode>
AssetsLibrary::createDrawableNodeById(const std::string& id, std::vector<vax::math::Transform> transforms) {
    uint32_t instancesCount = transforms.size();
    auto itModelInfo = _modelMap.find(id);
    if (itModelInfo != _modelMap.end()) {
        auto drawableNode = DrawableNode(_resourceManager.get().ssboManager(), id, transforms, true);
        if (itModelInfo->second.isIdentifiable) {
            drawableNode.setNodeId(_lastObjectId);
            _lastObjectId += instancesCount;
        }
        auto drawableModelPtr = &_drawableModels[itModelInfo->second.modelIndex];
        DrawableModelHandle drawableModelHandle = {drawableModelPtr};
        auto& chunkInfo = itModelInfo->second.ssboChunkInfos[itModelInfo->second.ssboChunkCursor];
        if (chunkInfo.isFull()) {
            ++itModelInfo->second.ssboChunkCursor;
            ModelInfo::SSBOChunkInfo newChunk = {
                .instanceOffset = _globalInstanceCursor,
                .cursor = instancesCount,
                .maxInstances = instancesCount + 10,
            };
            _modelMap[id].ssboChunkInfos.push_back(newChunk);
            drawableModelHandle.instanceDrawingRanges.push_back({newChunk.instanceOffset, instancesCount});
            _globalInstanceCursor += newChunk.maxInstances;
        } else {
            int leftInstancesCount = static_cast<int>(instancesCount);
            auto chunkCanTake =
                std::min(static_cast<int>(chunkInfo.maxInstances - chunkInfo.cursor), leftInstancesCount);
            drawableModelHandle.instanceDrawingRanges.push_back(
                {chunkInfo.instanceOffset + chunkInfo.cursor, chunkCanTake}
            );
            chunkInfo.cursor += chunkCanTake;
            leftInstancesCount -= static_cast<int>(chunkCanTake);
            if (chunkInfo.isFull() && leftInstancesCount > 0) {
                ++itModelInfo->second.ssboChunkCursor;
                ModelInfo::SSBOChunkInfo newChunk = {
                    .instanceOffset = _globalInstanceCursor,
                    .cursor = static_cast<uint32_t>(leftInstancesCount),
                    .maxInstances = static_cast<uint32_t>(leftInstancesCount + 10),
                };
                _modelMap[id].ssboChunkInfos.push_back(newChunk);
                drawableModelHandle.instanceDrawingRanges.push_back({newChunk.instanceOffset, leftInstancesCount});
                _globalInstanceCursor += newChunk.maxInstances;
            }
        }
        drawableNode.addDrawableModel(drawableModelHandle);
        return std::optional<DrawableNode>(std::in_place, std::move(drawableNode));
    }
    return std::nullopt;
}

std::optional<DrawableNode>
AssetsLibrary::getPreloadedDrawableNodeById(const std::string& id, uint32_t instancesCount) {
    auto itCachedDrawableNode = _cachedDrawableNodeMap.find(id);
    if (itCachedDrawableNode != _cachedDrawableNodeMap.end()) {
        return std::optional<DrawableNode>(std::in_place, std::move(itCachedDrawableNode->second));
    }
    return std::nullopt;
}

DrawableModelHandle AssetsLibrary::_addDrawableModel(std::string id, DrawableModel&& drawableModel) {
    auto itModelInfo = _modelMap.find(id);
    if (itModelInfo != _modelMap.end()) {
        // TODO: handle multiple instances
        auto chunkIndex = itModelInfo->second.ssboChunkCursor;
        auto ssboChunkInfo = itModelInfo->second.ssboChunkInfos[chunkIndex];
        std::vector<std::pair<uint32_t, uint32_t>> instanceDrawingRanges = {{{ssboChunkInfo.instanceOffset, 1}}};
        return DrawableModelHandle{&_drawableModels[itModelInfo->second.modelIndex], instanceDrawingRanges};
    }
    size_t modelIndex = _drawableModels.size();
    _drawableModels.push_back(std::move(drawableModel));
    ModelInfo::SSBOChunkInfo ssboChunkInfo = {
        .instanceOffset = _globalInstanceCursor,
        .cursor = 0,
        .maxInstances = 1,
    };
    ModelInfo modelInfo = {
        .modelIndex = modelIndex,
        .ssboChunkInfos = {ssboChunkInfo},
    };
    _globalInstanceCursor += 1;
    _modelMap[id] = modelInfo;
    std::vector<std::pair<uint32_t, uint32_t>> instanceDrawingRanges = {{{_globalInstanceCursor, 1}}};
    return DrawableModelHandle{&_drawableModels.back(), instanceDrawingRanges};
}