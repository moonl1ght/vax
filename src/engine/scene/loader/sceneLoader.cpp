#include "sceneLoader.h"
#include "environmentMap.h"
#include "prefabSpawner.h"
#include "resourceManager.h"
#include "sceneDescriptor.h"
#include <fstream>
#include <nlohmann/json.hpp>

using namespace vax::engine;
using namespace vax::ecs;

std::optional<SceneLoader::SceneWorldPair> SceneLoader::load(const std::string& path) {
    auto sceneDescriptor = _loadSceneDescriptor(path);
    if (!sceneDescriptor.has_value()) {
        return std::nullopt;
    }
    return load(sceneDescriptor.value());
}

SceneLoader::SceneWorldPair SceneLoader::load(SceneDescriptor& descriptor) {
    std::shared_ptr<ecs::World> world = std::make_shared<ecs::World>();
    auto scene = _initScene(*world, descriptor);
    scene->load(descriptor);
    scene->setWorld(world);
    return std::make_pair(std::move(scene), std::move(world));
}

std::optional<SceneDescriptor> SceneLoader::_loadSceneDescriptor(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }
    nlohmann::json json;
    file >> json;
    SceneDescriptor descriptor = json.get<SceneDescriptor>();
    return descriptor;
}

std::unique_ptr<Scene> SceneLoader::_initScene(ecs::World& world, const SceneDescriptor& descriptor) {
    std::unique_ptr<ecs::World> gizmoWorld = std::make_unique<ecs::World>();
    std::unique_ptr<ecs::World> backgroundWorld = std::make_unique<ecs::World>();
    auto maxDrawableInstances = vax::vk::MAX_DRAWABLE_INSTANCES;
    auto resourceManager = std::make_unique<vax::vk::ResourceManager>(*_vkEngine.get().device);
    resourceManager->setup(maxDrawableInstances);
    auto environmentMap = std::make_unique<EnvironmentMap>(*_vkEngine.get().device, *resourceManager);
    auto assetsLibrary =
        std::make_unique<AssetsLibrary>(*_vkEngine.get().device, maxDrawableInstances, *resourceManager);

    environmentMap->load({
        .textures = {
        {engine::EnvironmentMap::TextureType::BRDFLUT, RES_PATH("brdf/brdfLUT.ktx")},
        {engine::EnvironmentMap::TextureType::EnvMapIrradiance, RES_PATH("brdf/irradiance.ktx")},
        {engine::EnvironmentMap::TextureType::EnvMap, RES_PATH("brdf/prefilter.ktx")},
        },
    });

    PrefabDescriptor backgroundDescriptor = {
        .prefabType = vax::engine::PrefabDescriptor::PrefabType::PRESET,
        .id = "background",
        .primitiveDescriptor = vax::engine::PrefabDescriptor::PrimitiveDescriptor{
        .primitiveType = vax::engine::PrefabDescriptor::PrimitiveType::PLANE,
        },
    };
    PrefabDescriptor gizmoDescriptor = {
        .prefabType = vax::engine::PrefabDescriptor::PrefabType::ASSET,
        .id = "gizmo",
        .assetDescriptor = vax::engine::PrefabDescriptor::AssetDescriptor{
        .path = RES_PATH("assets/models/gizmo.glb"),
        },
    };
    std::vector<vax::engine::PrefabDescriptor> prefabDescriptors;
    prefabDescriptors.reserve(descriptor.entities.size() + 2);
    for (const auto& entity : descriptor.entities) {
        prefabDescriptors.push_back(*entity.prefabDescriptor);
    }
    prefabDescriptors.push_back(backgroundDescriptor);
    prefabDescriptors.push_back(gizmoDescriptor);

    assetsLibrary->preload(prefabDescriptors);

    auto agentEntities = descriptor.forEachEntityOfType(engine::EntityDescriptor::Type::Agent);
    for (const auto& agentEntity : agentEntities) {
        if (const Prefab* agentPrefab = assetsLibrary->findPrefab(agentEntity.prefabDescriptor->id)) {
            auto agentTransform = agentEntity.prefabDescriptor->getTransformForInstance(0);
            Entity agent = PrefabSpawner::spawnPrefab(world, *agentPrefab, agentTransform);
            world.addCoreComponentFor<AgentComponent>(agent);
        }
    }

    auto envEntities = descriptor.forEachEntityOfType(engine::EntityDescriptor::Type::Environment);
    uint32_t envEntityTypeIndex = 0;
    for (const auto& envEntity : envEntities) {
        auto prefabDescriptor = envEntity.prefabDescriptor;
        const Prefab* prefab = assetsLibrary->findPrefab(prefabDescriptor->id);
        if (prefab == nullptr) {
            continue;
        }
        for (uint32_t instanceIndex = 0; instanceIndex < prefabDescriptor->instanceInfos.size(); ++instanceIndex) {
            Entity block =
                PrefabSpawner::spawnPrefab(world, *prefab, prefabDescriptor->getTransformForInstance(instanceIndex));
            world.addCoreComponentFor<InstanceComponent>(block, envEntityTypeIndex, instanceIndex);
        }
        ++envEntityTypeIndex;
    }

    if (const Prefab* backgroundPrefab = assetsLibrary->findPrefab("background")) {
        Entity background = PrefabSpawner::spawnPrefab(*backgroundWorld, *backgroundPrefab, vax::math::Transform());
        backgroundWorld->addCoreComponentFor<BackgroundComponent>(background);
    }

    if (const Prefab* gizmoPrefab = assetsLibrary->findPrefab("gizmo")) {
        for (const auto& node : gizmoPrefab->nodes) {
            if (node.model.has_value()) {
                assetsLibrary->drawableModel(*node.model).settings().precomputedMVP = true;
            }
        }
        Entity gizmo = PrefabSpawner::spawnPrefab(*gizmoWorld, *gizmoPrefab, vax::math::Transform());
        gizmoWorld->addCoreComponentFor<GizmoComponent>(gizmo);
    }

    auto sceneComposer =
        std::make_unique<SceneComposer>(std::move(assetsLibrary), resourceManager->ssboManager(), maxDrawableInstances);

    return std::make_unique<Scene>(
        _vkEngine.get(),
        std::move(gizmoWorld),
        std::move(backgroundWorld),
        std::move(resourceManager),
        std::move(environmentMap),
        std::move(sceneComposer)
    );
}