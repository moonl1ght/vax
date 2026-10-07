#include "sceneLoader.h"
#include "environmentMap.h"
#include "prefabSpawner.h"
#include "resourceManager.h"

using namespace vax::engine;
using namespace vax::ecs;

std::unique_ptr<Scene>
SceneLoader::load(const std::string& path, const vax::rl::GridWorldDrawableDescriptor& descriptor) {
    std::unique_ptr<ecs::World> world = std::make_unique<ecs::World>();
    auto drawableScene = _initDrawableScene(*world, descriptor);
    _loadSceneAndWorld(world, drawableScene, descriptor);
    return std::make_unique<Scene>(std::move(world), std::move(drawableScene));
}

std::unique_ptr<DrawableScene>
SceneLoader::_initDrawableScene(ecs::World& world, const vax::rl::GridWorldDrawableDescriptor& descriptor) {
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
        .id = "background",
        .primitiveDescriptor =
            vax::engine::PrefabDescriptor::PrimitiveDescriptor{
            .primitiveType = vax::engine::PrefabDescriptor::PrimitiveType::PLANE,
            },
        .prefabType = vax::engine::PrefabDescriptor::PrefabType::PRESET,
    };
    PrefabDescriptor gizmoDescriptor = {
        .id = "gizmo",
        .assetDescriptor =
            vax::engine::PrefabDescriptor::AssetDescriptor{
            .path = RES_PATH("assets/models/gizmo.glb"),
            },
        .prefabType = vax::engine::PrefabDescriptor::PrefabType::ASSET,
    };
    std::vector<vax::engine::PrefabDescriptor> prefabDescriptors = descriptor.drawableDescriptors;
    prefabDescriptors.push_back(descriptor.agentDrawableDescriptor);
    prefabDescriptors.push_back(backgroundDescriptor);
    prefabDescriptors.push_back(gizmoDescriptor);

    assetsLibrary->preloadv2(prefabDescriptors);

    const auto& agentDescriptor = descriptor.agentDrawableDescriptor;
    if (const Prefab* agentPrefab = assetsLibrary->findPrefab(agentDescriptor.id)) {
        auto agentTransform =
            agentDescriptor.transforms.empty() ? vax::math::Transform() : agentDescriptor.transforms.front();
        Entity agent = PrefabSpawner::spawnPrefab(world, *agentPrefab, agentTransform);
        world.addComponentFor<AgentComponent>(agent);
    }

    for (uint32_t typeIndex = 0; typeIndex < descriptor.drawableDescriptors.size(); ++typeIndex) {
        const auto& blockDescriptor = descriptor.drawableDescriptors[typeIndex];
        const Prefab* prefab = assetsLibrary->findPrefab(blockDescriptor.id);
        if (prefab == nullptr) {
            continue;
        }
        for (uint32_t instanceIndex = 0; instanceIndex < blockDescriptor.transforms.size(); ++instanceIndex) {
            Entity block = PrefabSpawner::spawnPrefab(world, *prefab, blockDescriptor.transforms[instanceIndex]);
            world.addComponentFor<InstanceComponent>(block, typeIndex, instanceIndex);
        }
    }

    if (const Prefab* backgroundPrefab = assetsLibrary->findPrefab("background")) {
        Entity background = PrefabSpawner::spawnPrefab(*backgroundWorld, *backgroundPrefab, vax::math::Transform());
        backgroundWorld->addComponentFor<BackgroundComponent>(background);
    }

    if (const Prefab* gizmoPrefab = assetsLibrary->findPrefab("gizmo")) {
        Entity gizmo = PrefabSpawner::spawnPrefab(*gizmoWorld, *gizmoPrefab, vax::math::Transform());
        gizmoWorld->addComponentFor<GizmoComponent>(gizmo);
    }

    return std::make_unique<DrawableScene>(
        _vkEngine.get(),
        world,
        std::move(gizmoWorld),
        std::move(backgroundWorld),
        std::move(drawableWorld),
        std::move(resourceManager),
        std::move(assetsLibrary),
        std::move(environmentMap)
    );
}

void SceneLoader::_loadSceneAndWorld(
    std::unique_ptr<ecs::World>& world,
    std::unique_ptr<DrawableScene>& drawableScene,
    const vax::rl::GridWorldDrawableDescriptor& descriptor
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

    drawableScene->_drawableWorld->load(*drawableScene->_assetsLibrary, descriptor);

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