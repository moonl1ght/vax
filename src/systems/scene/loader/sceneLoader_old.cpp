#include "sceneLoader_old.h"
#include "environmentMap.h"
#include "paths.h"
#include "physicsComponents.h"
#include "prefabSpawner.h"
#include "resourceManager.h"
#include "sceneConfig.h"
#include "textureLoader.h"
#include <fstream>
#include <nlohmann/json.hpp>

using namespace vax;
using namespace vax::gfx;
using namespace vax::ecs;

std::optional<SceneLoader::SceneWorldPair> SceneLoader::load(const std::string& path) {
    auto sceneDescriptor = _loadSceneConfig(path);
    if (!sceneDescriptor.has_value()) {
        return std::nullopt;
    }
    return load(sceneDescriptor.value());
}

SceneLoader::SceneWorldPair SceneLoader::load(scene::SceneConfig& descriptor) {
    std::shared_ptr<ecs::World> world = std::make_shared<ecs::World>();
    auto scene = _initScene(*world, descriptor);
    scene->load(descriptor);
    scene->setWorld(world);
    auto& loadQueue = _vkEngine.get().queueManager->getMainGraphicsQueue();
    auto loadCommandBuffer = _vkEngine.get().commandManager->createSingleTimeCommandBuffer();

    loadCommandBuffer.begin();

    _textureGpuUploader->uploadTextures(loadCommandBuffer);

    loadCommandBuffer.end();
    loadCommandBuffer.submitAndWait(loadQueue);

    _textureGpuUploader->resetAllTexturesStagingBuffers();
    return std::make_pair(std::move(scene), std::move(world));
}

std::optional<scene::SceneConfig> SceneLoader::_loadSceneConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }
    nlohmann::json json;
    file >> json;
    scene::SceneConfig descriptor = json.get<scene::SceneConfig>();
    return descriptor;
}

std::unique_ptr<Scene> SceneLoader::_initScene(ecs::World& world, const scene::SceneConfig& descriptor) {
    std::unique_ptr<ecs::World> gizmoWorld = std::make_unique<ecs::World>();
    std::unique_ptr<ecs::World> backgroundWorld = std::make_unique<ecs::World>();
    auto maxDrawableInstances = vax::rhi::MAX_DRAWABLE_INSTANCES;
    _resourceManager.get().load(maxDrawableInstances);
    auto environmentMap = std::make_unique<EnvironmentMap>(*_vkEngine.get().device);
    auto assetsLibrary = std::make_unique<AssetsLibrary>(
        *_vkEngine.get().device,
        maxDrawableInstances,
        _resourceManager.get(),
        *_textureGpuUploader,
        _pipelineManager.get().descriptorRegistry()
    );

    auto textureLoader = rhi::TextureLoader(*_vkEngine.get().device, _pipelineManager.get().descriptorRegistry());
    environmentMap->load(
        {
        .textures =
            {
            {gfx::EnvironmentMap::TextureType::BRDFLUT, RES_PATH("brdf/brdfLUT.ktx")},
            {gfx::EnvironmentMap::TextureType::EnvMapIrradiance, RES_PATH("brdf/irradiance.ktx")},
            {gfx::EnvironmentMap::TextureType::EnvMap, RES_PATH("brdf/prefilter.ktx")},
            },
        },
        _resourceManager.get(),
        textureLoader,
        *_textureGpuUploader
    );

    scene::PrefabConfig backgroundConfig = {
        .prefabType = scene::PrefabConfig::PrefabType::PRESET,
        .id = "background",
        .primitiveDescriptor = scene::PrefabConfig::PrimitiveDescriptor{
        .primitiveType = scene::PrefabConfig::PrimitiveType::PLANE,
        },
    };
    scene::PrefabConfig gizmoConfig = {
        .prefabType = scene::PrefabConfig::PrefabType::ASSET,
        .id = "gizmo",
        .assetDescriptor = scene::PrefabConfig::AssetDescriptor{
        .path = RES_PATH("assets/models/gizmo.glb"),
        },
    };
    std::vector<scene::PrefabConfig> prefabConfigs;
    prefabConfigs.reserve(descriptor.entities.size() + 2);
    for (const auto& entity : descriptor.entities) {
        prefabConfigs.push_back(*entity.prefabConfig);
    }
    prefabConfigs.push_back(backgroundConfig);
    prefabConfigs.push_back(gizmoConfig);

    assetsLibrary->preload(prefabConfigs);

    auto agentEntities = descriptor.forEachEntityOfType(scene::EntityConfig::Type::Agent);
    for (const auto& agentEntity : agentEntities) {
        if (const Prefab* agentPrefab = assetsLibrary->findPrefab(agentEntity.prefabConfig->id)) {
            auto agentTransform = agentEntity.prefabConfig->getTransformForInstance(0);
            Entity agent = PrefabSpawner::spawnPrefab(world, *agentPrefab, agentTransform);
            world.addCoreComponentFor<AgentComponent>(agent);
        }
    }

    auto envEntities = descriptor.forEachEntityOfType(scene::EntityConfig::Type::Environment);
    uint32_t envEntityTypeIndex = 0;
    for (const auto& envEntity : envEntities) {
        auto prefabConfig = envEntity.prefabConfig;
        const Prefab* prefab = assetsLibrary->findPrefab(prefabConfig->id);
        if (prefab == nullptr) {
            continue;
        }
        for (uint32_t instanceIndex = 0; instanceIndex < prefabConfig->instanceInfos.size(); ++instanceIndex) {
            Entity entity =
                PrefabSpawner::spawnPrefab(world, *prefab, prefabConfig->getTransformForInstance(instanceIndex));
            world.addCoreComponentFor<InstanceComponent>(entity, envEntityTypeIndex, instanceIndex);
            if (prefabConfig->physicsDescriptor.has_value()) {
                if (prefabConfig->physicsDescriptor->isStatic) {
                    StaticRigidBodyComponent staticRigidBodyComponent =
                        prefabConfig->physicsDescriptor->getStaticRigidBodyComponent();
                    world.addCoreComponentFor<StaticRigidBodyComponent>(entity, staticRigidBodyComponent);
                } else {
                    RigidBodyComponent rigidBodyComponent = prefabConfig->physicsDescriptor->getRigidBodyComponent();
                    world.addCoreComponentFor<RigidBodyComponent>(entity, rigidBodyComponent);
                }
                switch (prefabConfig->physicsDescriptor->colliderType) {
                case scene::PhysicsObjectConfig::ColliderType::SPHERE: {
                    if (prefabConfig->physicsDescriptor->sphereColliderDescriptor.has_value()) {
                        SphereColliderComponent sphereColliderComponent =
                            prefabConfig->physicsDescriptor->sphereColliderDescriptor->getSphereColliderComponent();
                        world.addCoreComponentFor<SphereColliderComponent>(entity, sphereColliderComponent);
                    } else {
                        _logger.error("Sphere collider descriptor is not set for entity: ", prefabConfig->id);
                        continue;
                    }
                    break;
                }
                case scene::PhysicsObjectConfig::ColliderType::PLANE: {
                    if (prefabConfig->physicsDescriptor->planeColliderDescriptor.has_value()) {
                        PlaneColliderComponent planeColliderComponent =
                            prefabConfig->physicsDescriptor->planeColliderDescriptor->getPlaneColliderComponent();
                        world.addCoreComponentFor<PlaneColliderComponent>(entity, planeColliderComponent);
                    } else {
                        _logger.error("Plane collider descriptor is not set for entity: ", prefabConfig->id);
                        continue;
                    }
                    break;
                }
                };
            }
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

    auto sceneComposer = std::make_unique<SceneComposer>(
        std::move(assetsLibrary), _resourceManager.get().ssboManager(), maxDrawableInstances
    );

    return std::make_unique<Scene>(
        _vkEngine.get(),
        _windowController.get(),
        std::move(gizmoWorld),
        std::move(backgroundWorld),
        std::move(environmentMap),
        std::move(sceneComposer),
        _resourceManager.get()
    );
}
