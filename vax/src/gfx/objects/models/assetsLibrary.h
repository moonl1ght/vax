#pragma once

#include "drawableModel.h"
#include "logger.h"
#include "modelLoader.h"
#include "prefab.h"
#include "prefabDescriptor.h"
#include "prefabLoader.h"
#include "resourceManager.h"

namespace vax::gfx {
class AssetsLibrary {
  public:
    friend class ModelLoader;

    explicit AssetsLibrary(
        const vax::rhi::Device& device, uint32_t maxDrawableInstances, vax::rhi::ResourceManager& resourceManager
    )
        : _resourceManager(resourceManager)
        , _maxDrawableInstances(maxDrawableInstances) {
        _prefabLoader = std::make_unique<vax::gfx::PrefabLoader>(device, resourceManager);
        _drawableModels.reserve(_maxDrawableInstances);
    };

    ~AssetsLibrary() {};

    AssetsLibrary(const AssetsLibrary& other) = delete;
    AssetsLibrary& operator=(const AssetsLibrary& other) = delete;
    AssetsLibrary(AssetsLibrary&& other) noexcept = delete;
    AssetsLibrary& operator=(AssetsLibrary&& other) noexcept = delete;

    void preload(const std::vector<vax::gfx::PrefabDescriptor>& descriptors);

    std::vector<std::string> getDrawableNodeIds() const;

    const Prefab* findPrefab(const std::string& id) const {
        return _prefabs.find(id) != _prefabs.end() ? &_prefabs.at(id) : nullptr;
    }

    DrawableModel& drawableModel(vax::ecs::DrawableModelId id) { return _drawableModels[id]; }

    size_t drawableModelCount() const { return _drawableModels.size(); }

  private:
    vax::Logger _logger = vax::Logger("AssetsLibrary");
    const uint32_t _maxDrawableInstances;

    std::reference_wrapper<vax::rhi::ResourceManager> _resourceManager;

    std::unique_ptr<vax::gfx::PrefabLoader> _prefabLoader;

    std::unordered_map<std::string, vax::gfx::Prefab> _prefabs;

    std::vector<vax::gfx::DrawableModel> _drawableModels;
};
} // namespace vax::gfx