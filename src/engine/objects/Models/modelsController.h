#pragma once

#include "drawableModel.h"
#include "drawableNode.h"
#include "logger.h"
#include "modelDescriptor.h"
#include "modelLoader.h"
#include "primitivesBuilder.h"
#include "resourceManager.h"
#include "vkUtils.h"

namespace vax::engine {
class ModelsController {
  public:
    friend class ModelLoader;

    explicit ModelsController(
        const vax::vk::Device& device, uint32_t maxDrawableInstances, vax::vk::ResourceManager& resourceManager
    )
        : _resourceManager(resourceManager)
        , _maxDrawableInstances(maxDrawableInstances) {
        _modelLoader = std::make_unique<vax::engine::ModelLoader>(device, resourceManager, *this);
        _primitivesBuilder = std::make_unique<vax::engine::PrimitivesBuilder>(resourceManager);
        _drawableModels.reserve(_maxDrawableInstances);
    };

    ~ModelsController() {};

    ModelsController(const ModelsController& other) = delete;
    ModelsController& operator=(const ModelsController& other) = delete;
    ModelsController(ModelsController&& other) noexcept = delete;
    ModelsController& operator=(ModelsController&& other) noexcept = delete;

    void
    preload(const std::vector<vax::engine::ModelDescriptor>& modelDescriptors);

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

    vax::Logger _logger = vax::Logger("ModelsController");
    const uint32_t _maxDrawableInstances;

    std::reference_wrapper<vax::vk::ResourceManager> _resourceManager;

    std::unique_ptr<vax::engine::ModelLoader> _modelLoader;
    std::unique_ptr<vax::engine::PrimitivesBuilder> _primitivesBuilder;

    std::unordered_map<std::string, vax::engine::DrawableNode> _cachedDrawableNodeMap;

    std::unordered_map<std::string, ModelInfo> _modelMap;
    std::vector<vax::engine::DrawableModel> _drawableModels;

    uint32_t _globalInstanceCursor = 0;
    uint32_t _lastObjectId = 1;

    DrawableModelHandle _addDrawableModel(std::string id, std::string path, vax::engine::DrawableModel&& drawableModel);
};
} // namespace vax::engine