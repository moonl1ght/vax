#include "sceneLoader.h"
#include "environmentMap.h"
#include "resourceManager.h"

using namespace vax::engine;

std::unique_ptr<DrawableScene>
SceneLoader::load(const std::string& path, const vax::rl::GridWorldDrawableDescriptor& descriptor) {
    auto drawableScene = _initScene();
    _loadScene(drawableScene, descriptor);

    return drawableScene;
}

std::unique_ptr<DrawableScene> SceneLoader::_initScene() {
    auto maxDrawableInstances = vax::vk::MAX_DRAWABLE_INSTANCES;
    auto resourceManager = std::make_unique<vax::vk::ResourceManager>(*_vkEngine.get().device);
    resourceManager->setup(maxDrawableInstances);
    auto environmentMap = std::make_unique<EnvironmentMap>(*_vkEngine.get().device, *resourceManager);
    auto modelsController =
        std::make_unique<ModelsController>(*_vkEngine.get().device, maxDrawableInstances, *resourceManager);

    return std::make_unique<DrawableScene>(
        _vkEngine.get(), std::move(resourceManager), std::move(modelsController), std::move(environmentMap)
    );
}

void SceneLoader::_loadScene(
    std::unique_ptr<DrawableScene>& scene, const vax::rl::GridWorldDrawableDescriptor& descriptor
) {

    scene->_environmentMap->load({
        .textures = {
        {engine::EnvironmentMap::TextureType::BRDFLUT, RES_PATH("brdf/brdfLUT.ktx")},
        {engine::EnvironmentMap::TextureType::EnvMapIrradiance, RES_PATH("brdf/irradiance.ktx")},
        {engine::EnvironmentMap::TextureType::EnvMap, RES_PATH("brdf/prefilter.ktx")},
        },
    });

    scene->_sceneUniformBuffers.reserve(vax::vk::MAX_FRAMES_IN_FLIGHT);
    scene->_roverCameraUniformBuffers.reserve(vax::vk::MAX_FRAMES_IN_FLIGHT);
    scene->_lightsUniformBuffer.reserve(vax::vk::MAX_FRAMES_IN_FLIGHT);

    uint32_t lightCount = 1;
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    for (size_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto& bufferManager = scene->_resourceManager->bufferManager();
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
        scene->_sceneUniformBuffers.push_back(allocation.second);
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
        scene->_roverCameraUniformBuffers.push_back(roverCameraAllocation.second);
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
        scene->_lightsUniformBuffer.push_back(lightAllocation.second);
    }

    scene->_indirectDrawController->setup(10000);

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

    scene->_modelsController->preload(modelDescriptors);
    scene->_sceneGraph->load(*scene->_modelsController, descriptor);

    scene->_background = std::move(scene->_modelsController->createDrawableNodeById("background"));

    auto loadQueue = _vkEngine.get().queueManager->graphicsQueue;
    auto loadCommandBuffer = _vkEngine.get().commandManager->createSingleTimeCommandBuffer();

    loadCommandBuffer.begin();

    scene->_resourceManager->textureManager().loadAllTextures(loadCommandBuffer);

    loadCommandBuffer.end();
    loadCommandBuffer.submitAndWait(loadQueue);

    scene->_resourceManager->textureManager().resetAllTexturesStagingBuffers();

    auto sunCamera = Camera();
    sunCamera.setPosition(glm::vec3(1.0f, 5.0f, 3.0f));
    auto swapchainExtent = _vkEngine.get().getWindowController().getWindow(0)->getSwapchain()->swapchainExtent;
    sunCamera.setViewPortSize(vax::math::SizeUI(swapchainExtent));
    sunCamera.setProjection(Camera::Projection::orthographic);
    sunCamera.setViewSize(10.0f);
    scene->_sunLight = Light(sunCamera);
    scene->_sunLight.setLightUBOIndex(0);

    auto cameraPos = glm::vec3(1.0f, 5.0f, -3.0f);
    scene->_mainCamera.setPosition(cameraPos);
}