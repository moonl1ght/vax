#pragma once

#include "drawableModel.h"
#include "drawableNode.h"
#include "logger.h"
#include "modelDescriptor.h"
#include "modelLoader.h"
#include "prefabLoader.h"
#include "primitivesBuilder.h"
#include "resourceManager.h"

namespace vax::engine {
class AssetsLibrary {
  public:
    friend class ModelLoader;

    explicit AssetsLibrary(
        const vax::vk::Device& device, uint32_t maxDrawableInstances, vax::vk::ResourceManager& resourceManager
    )
        : _resourceManager(resourceManager)
        , _maxDrawableInstances(maxDrawableInstances) {
        _modelLoader = std::make_unique<vax::engine::ModelLoader>(device, resourceManager, *this);
        _primitivesBuilder = std::make_unique<vax::engine::PrimitivesBuilder>(resourceManager);
        _prefabLoader = std::make_unique<vax::engine::PrefabLoader>(*_modelLoader, resourceManager);
        _drawableModels.reserve(_maxDrawableInstances);
    };

    ~AssetsLibrary() {};

    AssetsLibrary(const AssetsLibrary& other) = delete;
    AssetsLibrary& operator=(const AssetsLibrary& other) = delete;
    AssetsLibrary(AssetsLibrary&& other) noexcept = delete;
    AssetsLibrary& operator=(AssetsLibrary&& other) noexcept = delete;

    void preload(const std::vector<vax::engine::ModelDescriptor>& modelDescriptors);

    void preloadv2(const std::vector<vax::engine::ModelDescriptor>& modelDescriptors);

    std::vector<std::string> getModelIds() const;

    std::vector<std::string> getDrawableNodeIds() const;

    std::optional<vax::engine::DrawableNode>
    getPreloadedDrawableNodeById(const std::string& id, uint32_t instancesCount);

    std::optional<vax::engine::DrawableNode> createDrawableNodeById(
        const std::string& id, std::vector<vax::math::Transform> transforms = {vax::math::Transform()}
    );

  private:
    struct ModelInfo final {
        struct SSBOChunkInfo final {
            uint32_t instanceOffset;
            uint32_t cursor;
            uint32_t maxInstances;

            bool isFull() const { return cursor >= maxInstances; }
        };

        size_t modelIndex;
        std::vector<SSBOChunkInfo> ssboChunkInfos;
        uint32_t ssboChunkCursor = 0;
        bool isIdentifiable = true;
    };

    vax::Logger _logger = vax::Logger("AssetsLibrary");
    const uint32_t _maxDrawableInstances;

    std::reference_wrapper<vax::vk::ResourceManager> _resourceManager;

    std::unique_ptr<vax::engine::ModelLoader> _modelLoader;
    std::unique_ptr<vax::engine::PrimitivesBuilder> _primitivesBuilder;
    std::unique_ptr<vax::engine::PrefabLoader> _prefabLoader;

    std::unordered_map<std::string, vax::engine::DrawableNode> _cachedDrawableNodeMap;

    std::unordered_map<std::string, ModelInfo> _modelMap;
    std::vector<vax::engine::DrawableModel> _drawableModels;

    uint32_t _globalInstanceCursor = 0;
    uint32_t _lastObjectId = 1;

    DrawableModelHandle _addDrawableModel(std::string id, std::string path, vax::engine::DrawableModel&& drawableModel);
};
} // namespace vax::engine