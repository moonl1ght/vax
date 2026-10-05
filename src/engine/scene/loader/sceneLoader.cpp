#include "sceneLoader.h"
#include "environmentMap.h"
#include "resourceManager.h"
#include "prefabSpawner.h"

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

    std::vector<vax::engine::ModelDescriptor> modelDescriptors = {
        {
        .path = "",
        .id = "background",
        .modelType = vax::engine::ModelDescriptor::ModelType::PRIMITIVE_PLANE,
        },
        {
        .path = RES_PATH("assets/models/gizmo.glb"),
        .id = "gizmo",
        .modelType = vax::engine::ModelDescriptor::ModelType::MODEL,
        }
    };
    for (const auto& drawableDescriptor : descriptor.drawableDescriptors) {
        modelDescriptors.push_back(drawableDescriptor);
    }
    modelDescriptors.push_back(descriptor.agentDrawableDescriptor);

    assetsLibrary->preload(modelDescriptors);
    assetsLibrary->preloadv2(modelDescriptors);

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

    return std::make_unique<DrawableScene>(
        _vkEngine.get(),
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

    drawableScene->_background = std::move(drawableScene->_assetsLibrary->createDrawableNodeById("background"));

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