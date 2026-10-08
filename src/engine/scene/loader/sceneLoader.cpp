#include "sceneLoader.h"
#include "environmentMap.h"
#include "prefabSpawner.h"
#include "resourceManager.h"
#include "sceneDescriptor.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

using namespace vax::engine;
using namespace vax::ecs;

std::unique_ptr<Scene> SceneLoader::load(const std::string& path) {
    auto sceneDescriptor = _loadSceneDescriptor(path);
    if (!sceneDescriptor.has_value()) {
        return nullptr;
    }
    return load(sceneDescriptor.value());
}

std::unique_ptr<Scene> SceneLoader::load(SceneDescriptor& descriptor) {
    std::unique_ptr<ecs::World> world = std::make_unique<ecs::World>();
    auto drawableScene = _initDrawableScene(*world, descriptor);
    _loadSceneAndWorld(world, drawableScene);
    return std::make_unique<Scene>(std::move(world), std::move(drawableScene));
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

std::unique_ptr<DrawableScene> SceneLoader::_initDrawableScene(ecs::World& world, const SceneDescriptor& descriptor) {
    std::unique_ptr<ecs::World> gizmoWorld = std::make_unique<ecs::World>();
    std::unique_ptr<ecs::World> backgroundWorld = std::make_unique<ecs::World>();
    auto drawableWorld = std::make_unique<vax::rl::GWDrawableWorld>(world);
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
            world.addComponentFor<AgentComponent>(agent);
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
            world.addComponentFor<InstanceComponent>(block, envEntityTypeIndex, instanceIndex);
        }
        ++envEntityTypeIndex;
    }

    if (const Prefab* backgroundPrefab = assetsLibrary->findPrefab("background")) {
        Entity background = PrefabSpawner::spawnPrefab(*backgroundWorld, *backgroundPrefab, vax::math::Transform());
        backgroundWorld->addComponentFor<BackgroundComponent>(background);
    }

    if (const Prefab* gizmoPrefab = assetsLibrary->findPrefab("gizmo")) {
        Entity gizmo = PrefabSpawner::spawnPrefab(*gizmoWorld, *gizmoPrefab, vax::math::Transform());
        gizmoWorld->addComponentFor<GizmoComponent>(gizmo);
    }

    drawableWorld->load(descriptor);

    auto sceneComposer =
        std::make_unique<SceneComposer>(std::move(assetsLibrary), resourceManager->ssboManager(), maxDrawableInstances);

    return std::make_unique<DrawableScene>(
        _vkEngine.get(),
        std::move(gizmoWorld),
        std::move(backgroundWorld),
        std::move(drawableWorld),
        std::move(resourceManager),
        std::move(environmentMap),
        std::move(sceneComposer)
    );
}

void SceneLoader::_loadSceneAndWorld(
    std::unique_ptr<ecs::World>& world, std::unique_ptr<DrawableScene>& drawableScene
) {

    drawableScene->_sceneUniformBuffers.reserve(vax::vk::MAX_FRAMES_IN_FLIGHT);
    drawableScene->_roverCameraUniformBuffers.reserve(vax::vk::MAX_FRAMES_IN_FLIGHT);
    drawableScene->_lightsUniformBuffer.reserve(vax::vk::MAX_FRAMES_IN_FLIGHT);

    uint32_t lightCount = 1;
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    for (size_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto& bufferManager = drawableScene->_resourceManager->bufferManager();
        auto passUboStride = _vkEngine.get().device->minUniformBufferOffsetAlignment<UniformBufferObject>();
        auto allocation = bufferManager
                              .allocateBuffer(
                                  "frame_uniform_buffer",
                                  passUboStride * (lightCount + 1),
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  VMA_MEMORY_USAGE_CPU_TO_GPU,
                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                              )
                              .value();
        allocation.second->map();
        drawableScene->_sceneUniformBuffers.push_back(allocation.second);
        auto roverCameraAllocation = bufferManager
                                         .allocateBuffer(
                                             "rover_camera_uniform_buffer",
                                             bufferSize,
                                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                             VMA_MEMORY_USAGE_CPU_TO_GPU,
                                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                         )
                                         .value();
        roverCameraAllocation.second->map();
        drawableScene->_roverCameraUniformBuffers.push_back(roverCameraAllocation.second);
        auto lightAllocation = bufferManager
                                   .allocateBuffer(
                                       "light_uniform_buffer",
                                       sizeof(LightUBO),
                                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                       VMA_MEMORY_USAGE_CPU_TO_GPU,
                                       VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                   )
                                   .value();
        lightAllocation.second->map();
        drawableScene->_lightsUniformBuffer.push_back(lightAllocation.second);
    }

    drawableScene->_indirectDrawController->setup(10000);

    auto loadQueue = _vkEngine.get().queueManager->graphicsQueue;
    auto loadCommandBuffer = _vkEngine.get().commandManager->createSingleTimeCommandBuffer();

    loadCommandBuffer.begin();

    drawableScene->_resourceManager->textureManager().loadAllTextures(loadCommandBuffer);

    loadCommandBuffer.end();
    loadCommandBuffer.submitAndWait(loadQueue);

    drawableScene->_resourceManager->textureManager().resetAllTexturesStagingBuffers();

    auto sunCamera = Camera();
    sunCamera.setPosition(glm::vec3(1.0f, 5.0f, 3.0f));
    auto swapchainExtent = _vkEngine.get().getWindowController().getWindow(0)->getSwapchain()->swapchainExtent;
    sunCamera.setViewPortSize(vax::math::SizeUI(swapchainExtent));
    sunCamera.setProjection(Camera::Projection::orthographic);
    sunCamera.setViewSize(10.0f);
    drawableScene->_sunLight = Light(sunCamera);
    drawableScene->_sunLight.setLightUBOIndex(0);

    auto cameraPos = glm::vec3(1.0f, 5.0f, -3.0f);
    drawableScene->_mainCamera.setPosition(cameraPos);
}