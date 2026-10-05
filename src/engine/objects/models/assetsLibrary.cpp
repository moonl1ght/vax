#include "assetsLibrary.h"
#include "transform.h"
#include <strings.h>

using namespace vax::engine;
using namespace vax::vk;
using namespace vax;

void ModelsController::preloadv2(const std::vector<vax::engine::ModelDescriptor>& modelDescriptors) {
    for (const auto& modelDescriptor : modelDescriptors) {
    }
}

void ModelsController::preload(const std::vector<vax::engine::ModelDescriptor>& modelDescriptors) {
    uint32_t totalModelsCount = 0;
    _globalInstanceCursor = 0;
    for (const auto& modelDescriptor : modelDescriptors) {
        totalModelsCount += modelDescriptor.instancesCount;
        if (totalModelsCount > _maxDrawableInstances) {
            _logger.error(
                "Total models count exceeds the maximum number of drawable instances: " +
                std::to_string(totalModelsCount)
            );
            break;
        }
        auto itModelInfo = _modelMap.find(modelDescriptor.id);
        if (itModelInfo != _modelMap.end()) {
            continue;
        }
        bool isURDF = false;
        switch (modelDescriptor.modelType) {
        case vax::engine::ModelDescriptor::ModelType::MODEL: {
            // TODO: refactor this with ECS adopting
            if (modelDescriptor.getModelExtension() == vax::engine::ModelDescriptor::ModelExtension::URDF) {
                auto drawableNode = _modelLoader->loadSceneModel(modelDescriptor);
                if (drawableNode) {
                    if (modelDescriptor.isIdentifiable) {
                        drawableNode->setNodeId(_lastObjectId);
                        _lastObjectId += modelDescriptor.instancesCount;
                    }
                    _cachedDrawableNodeMap.insert_or_assign(modelDescriptor.id, std::move(*drawableNode));
                    isURDF = true;
                }
            } else {
                auto model = _modelLoader->loadModel(modelDescriptor.path, modelDescriptor.instancesCount);
                if (model) {
                    _drawableModels.push_back(std::move(*model));
                }
            }
            break;
        }
        case vax::engine::ModelDescriptor::ModelType::PRIMITIVE_CUBE: {
            auto primitiveDescriptor = modelDescriptor.primitiveDescriptor;
            auto primitive = _primitivesBuilder->createCube(primitiveDescriptor.size, primitiveDescriptor.color);
            if (primitive) {
                _drawableModels.push_back(std::move(*primitive));
            }
            break;
        }
        case ModelDescriptor::ModelType::PRIMITIVE_PLANE: {
            auto primitive = _primitivesBuilder->createPlane();
            if (primitive) {
                _drawableModels.push_back(std::move(*primitive));
            }
            break;
        }
        }
        if (!isURDF) {
            auto instanceCount = modelDescriptor.instancesCount + 10;
            ModelInfo::SSBOChunkInfo ssboChunkInfo = {
                .instanceOffset = _globalInstanceCursor,
                .cursor = 0,
                .maxInstances = instanceCount,
            };
            ModelInfo modelInfo = {
                .modelIndex = _drawableModels.size() - 1,
                .ssboChunkInfos = {ssboChunkInfo},
                .ssboChunkCursor = 0,
                .isIdentifiable = modelDescriptor.isIdentifiable,
            };
            _modelMap[modelDescriptor.id] = modelInfo;
            _globalInstanceCursor += instanceCount;
        }
    }

    for (const auto& modelDescriptor : modelDescriptors) {
        auto prefab = _prefabLoader->loadPrefab(modelDescriptor);
    }
}

std::vector<std::string> ModelsController::getModelIds() const {
    std::vector<std::string> modelIds;
    modelIds.reserve(_modelMap.size());
    for (const auto& [id, _] : _modelMap) {
        modelIds.push_back(id);
    }
    return modelIds;
}

std::optional<DrawableNode>
ModelsController::createDrawableNodeById(const std::string& id, std::vector<vax::math::Transform> transforms) {
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
ModelsController::getPreloadedDrawableNodeById(const std::string& id, uint32_t instancesCount) {
    auto itCachedDrawableNode = _cachedDrawableNodeMap.find(id);
    if (itCachedDrawableNode != _cachedDrawableNodeMap.end()) {
        return std::optional<DrawableNode>(std::in_place, std::move(itCachedDrawableNode->second));
    }
    return std::nullopt;
}

DrawableModelHandle
ModelsController::_addDrawableModel(std::string id, std::string path, DrawableModel&& drawableModel) {
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