#include "scene.h"
#include "camera.h"
#include "profiler.h"
#include "swapchain.h"
#include "components.h"

using namespace vax;
using namespace vax::vk;
using namespace vax::engine;
using namespace vax::rl;

void Scene::load(const vax::engine::SceneDescriptor& descriptor) {
    _descriptor = descriptor;

    _renderPassGraphDescriptor.update([&](vax::engine::RenderPassGraphDescriptor& renderPassGraphDescriptor) {
        renderPassGraphDescriptor.withGizmo = descriptor.shouldDrawGizmo;
        renderPassGraphDescriptor.withSecondaryWindow = false;
    });

    _sceneUniformBuffers.reserve(vax::vk::MAX_FRAMES_IN_FLIGHT);
    _agentCameraUniformBuffers.reserve(vax::vk::MAX_FRAMES_IN_FLIGHT);
    _lightsUniformBuffer.reserve(vax::vk::MAX_FRAMES_IN_FLIGHT);

    uint32_t lightCount = 1;
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    for (size_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto& bufferManager = _resourceManager->bufferManager();
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
        _sceneUniformBuffers.push_back(allocation.second);
        auto agentCameraAllocation = bufferManager
                                         .allocateBuffer(
                                             "agent_camera_uniform_buffer",
                                             bufferSize,
                                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                             VMA_MEMORY_USAGE_CPU_TO_GPU,
                                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                         )
                                         .value();
        agentCameraAllocation.second->map();
        _agentCameraUniformBuffers.push_back(agentCameraAllocation.second);
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
        _lightsUniformBuffer.push_back(lightAllocation.second);
    }

    _indirectDrawController->setup(10000);

    auto loadQueue = _vkEngine.get().queueManager->graphicsQueue;
    auto loadCommandBuffer = _vkEngine.get().commandManager->createSingleTimeCommandBuffer();

    loadCommandBuffer.begin();

    _resourceManager->textureManager().loadAllTextures(loadCommandBuffer);

    loadCommandBuffer.end();
    loadCommandBuffer.submitAndWait(loadQueue);

    _resourceManager->textureManager().resetAllTexturesStagingBuffers();

    auto sunCamera = Camera();
    sunCamera.setPosition(glm::vec3(1.0f, 5.0f, 3.0f));
    auto swapchainExtent = _vkEngine.get().getWindowController().getWindow(0)->getSwapchain()->swapchainExtent;
    sunCamera.setViewPortSize(vax::math::SizeUI(swapchainExtent));
    sunCamera.setProjection(Camera::Projection::orthographic);
    sunCamera.setViewSize(10.0f);
    _sunLight = Light(sunCamera);
    _sunLight.setLightUBOIndex(0);

    auto cameraPos = glm::vec3(1.0f, 5.0f, -3.0f);
    _mainCamera.setPosition(cameraPos);
    _gizmoCamera.setViewPortSize(vax::math::SizeUI(256, 256));
    _gizmoCamera.setProjection(Camera::Projection::orthographic);
    _gizmoCamera.setPosition(cameraPos);
    _gizmoCamera.setViewSize(1.5f);

    _gizmoWorld->eachCoreComponentEntity<ecs::GizmoComponent>([&](ecs::Entity entity, ecs::GizmoComponent&) {
        _gizmoEntity = entity;
    });
}

void Scene::prepareForDraw(engine::RenderCallContext renderCallContext) {
    ZoneScopedN("DrawableScene::prepareForDraw");
    _renderCallContext = renderCallContext;
    if (auto mappedMemory = _sceneUniformBuffers[renderCallContext.currentFrame]->mappedMemory()) {
        uint8_t* mappedPtr = static_cast<uint8_t*>(mappedMemory.value());
        auto passUboStride = _vkEngine.get().device->minUniformBufferOffsetAlignment<UniformBufferObject>();
        memcpy(mappedPtr + (0 * passUboStride), &_ubo, sizeof(_ubo));
        memcpy(mappedPtr + (1 * passUboStride), &_sunLightUbo, sizeof(_sunLightUbo));
    } else {
        _logger.error("Failed to get mapped memory!");
    }

    if (auto mappedMemory = _agentCameraUniformBuffers[renderCallContext.currentFrame]->mappedMemory()) {
        memcpy(mappedMemory.value(), &_agentCameraUbo, sizeof(_agentCameraUbo));
    } else {
        _logger.error("Failed to get mapped memory!");
    }

    auto lightMappedMemory = _lightsUniformBuffer[renderCallContext.currentFrame]->mappedMemory();
    if (lightMappedMemory.has_value()) {
        auto lightData = static_cast<LightUBO*>(lightMappedMemory.value());
        lightData->lightCount = 1;
        auto sunLightUbo = _sunLight.lightUBOIndex();
        lightData->lights[sunLightUbo].lightSpaceMatrix =
            _sunLight.camera().projectionMatrix() * _sunLight.camera().viewMatrix();
        lightData->lights[sunLightUbo].position = glm::vec4(_sunLight.camera().position(), 1.0f);
        lightData->lights[sunLightUbo].color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        lightData->lights[sunLightUbo].shadowMapIndex = 0;
        lightData->lights[sunLightUbo].radius = 10.0f;
        lightData->lights[sunLightUbo].shadowMapSamplerIndex = 0;
    } else {
        _logger.error("Failed to get mapped memory!");
    }
}

void Scene::update(engine::SceneUpdateContext sceneUpdateContext) {
    if (!_world) {
        _logger.warning("ecs world is not set!");
        return;
    }
    _sceneUpdateContext = sceneUpdateContext;
    _ubo = _mainCamera.getUniformBufferObject();
    _ubo.environmentMapIndex = 0;
    _sceneComposer->updateTransforms(*_world);

    _agentCameraUbo = _agentCamera.getUniformBufferObject();
    _agentCameraUbo.environmentMapIndex = 0;
    _sunLightUbo = _sunLight.camera().getUniformBufferObject();
    _sunLightUbo.environmentMapIndex = 0;

    auto viewMatrix = _gizmoCamera.viewMatrix();
    auto projectionMatrix = _gizmoCamera.projectionMatrix();
    auto viewProjectionMatrix = projectionMatrix * viewMatrix;

    auto gizmoTransform = _gizmoWorld->getCoreComponentFor<ecs::WorldTransformComponent>(_gizmoEntity);
    if (gizmoTransform) {
        auto mvpMatrix = math::TransformMatrixHandle(viewProjectionMatrix);
        gizmoTransform->modelMatrix = mvpMatrix.getModelMatrix();
        gizmoTransform->normalMatrix = mvpMatrix.getNormalMatrix();
    }
}

void Scene::resize() {
    auto swapchainExtent = _vkEngine.get().getWindowController().getWindow(0)->getSwapchain()->swapchainExtent;
    _mainCamera.setViewPortSize(vax::math::SizeUI(swapchainExtent));
    _sunLight.camera().setViewPortSize(vax::math::SizeUI(swapchainExtent));
}

bool Scene::writePerDrawDescriptorSet(vax::vk::DescriptorSetWriter& descriptorWriter) {
    _indirectDrawController->writePerDrawDescriptorSet(descriptorWriter, _renderCallContext.currentFrame);
    return true;
}

bool Scene::writeGlobalDescriptorSet(vax::vk::DescriptorSetWriter& descriptorWriter) {
    auto globalSampler = _resourceManager->textureManager().getGlobalSampler(GlobalSampler::PBRSampler);
    auto globalCubeMapSampler = _resourceManager->textureManager().getGlobalSampler(GlobalSampler::CubeMapSampler);
    if (!globalSampler.has_value() || !globalCubeMapSampler.has_value()) {
        return false;
    }
    descriptorWriter.writeBuffer(
        _resourceManager->materialManager().materialBuffer(),
        GlobalDescriptorSetResourceIndex::GLOBAL_MATERIAL_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    );
    descriptorWriter.writeBuffer(
        _environmentMap->environmentMapBuffer(),
        GlobalDescriptorSetResourceIndex::GLOBAL_ENVIRONMENT_MAP_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    );
    descriptorWriter.writeSampler(*globalSampler->second, GlobalDescriptorSetResourceIndex::GLOBAL_SAMPLER_INDEX, 0);
    descriptorWriter.writeSampler(
        *globalCubeMapSampler->second, GlobalDescriptorSetResourceIndex::GLOBAL_SAMPLER_INDEX, 1
    );
    _resourceManager->textureManager().updateDescriptorWriterWithAllTextures(
        descriptorWriter, GlobalDescriptorSetResourceIndex::GLOBAL_TEXTURE_INDEX
    );
    return true;
}

bool Scene::writeFrameDescriptorSet(
    vax::vk::DescriptorSetWriter& descriptorWriter, vax::vk::DescriptorSetWriter& agentCameraDescriptorWriter
) {
    descriptorWriter.writeBuffer(
        *_sceneUniformBuffers[_renderCallContext.currentFrame],
        PerFrameDescriptorSetResourceIndex::FRAME_UNIFORM_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    );
    descriptorWriter.writeBuffer(
        *_lightsUniformBuffer[_renderCallContext.currentFrame],
        PerFrameDescriptorSetResourceIndex::FRAME_LIGHT_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    );
    descriptorWriter.writeBuffer(
        _resourceManager->ssboManager().instanceBuffer(_renderCallContext.currentFrame),
        PerFrameDescriptorSetResourceIndex::FRAME_INSTANCE_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    );

    agentCameraDescriptorWriter.writeBuffer(
        *_agentCameraUniformBuffers[_renderCallContext.currentFrame],
        PerFrameDescriptorSetResourceIndex::FRAME_UNIFORM_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    );
    agentCameraDescriptorWriter.writeBuffer(
        *_lightsUniformBuffer[_renderCallContext.currentFrame],
        PerFrameDescriptorSetResourceIndex::FRAME_LIGHT_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    );
    agentCameraDescriptorWriter.writeBuffer(
        _resourceManager->ssboManager().instanceBuffer(_renderCallContext.currentFrame),
        PerFrameDescriptorSetResourceIndex::FRAME_INSTANCE_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    );
    return true;
}

void Scene::draw(const DrawContext& drawContext) {
    VkBuffer vertexBuffers[] = {_resourceManager->meshManager().globalVertexBuffer(0)};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(drawContext.commandBuffer.vkCommandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(
        drawContext.commandBuffer.vkCommandBuffer,
        _resourceManager->meshManager().globalIndexBuffer(0),
        0,
        VK_INDEX_TYPE_UINT32
    );

    auto perDrawDescriptorSetHandler = _vkEngine.get().descriptorSetManager->getDescriptorSetHandler(
        CommonDescriptorSetName::PER_DRAW, drawContext.currentFrame
    );
    if (!perDrawDescriptorSetHandler.has_value()) {
        _logger.error("Failed to get per draw descriptor set handler!");
        return;
    }
    perDrawDescriptorSetHandler->bind(
        drawContext.commandBuffer.vkCommandBuffer, drawContext.pipelineLayout, MainSetIndices::PER_DRAW_SET_INDEX
    );

    if (auto drawRange = _indirectDrawController->drawRange("scene")) {
        GlobalPushConstants pushConstants{.drawIndexOffset = drawRange->start};
        vkCmdPushConstants(
            drawContext.commandBuffer.vkCommandBuffer,
            drawContext.pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(pushConstants),
            &pushConstants
        );
        _indirectDrawController->drawRange(drawContext.commandBuffer, drawContext.currentFrame, "scene");
    }
}

void Scene::drawBackground(const DrawContext& drawContext) {
    VkBuffer vertexBuffers[] = {_resourceManager->meshManager().globalVertexBuffer(0)};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(drawContext.commandBuffer.vkCommandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(
        drawContext.commandBuffer.vkCommandBuffer,
        _resourceManager->meshManager().globalIndexBuffer(0),
        0,
        VK_INDEX_TYPE_UINT32
    );
    _indirectDrawController->drawRange(drawContext.commandBuffer, drawContext.currentFrame, "background");
}

void Scene::drawGizmo(const DrawContext& drawContext) {
    if (!_descriptor->shouldDrawGizmo)
        return;
    VkBuffer vertexBuffers[] = {_resourceManager->meshManager().globalVertexBuffer(0)};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(drawContext.commandBuffer.vkCommandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(
        drawContext.commandBuffer.vkCommandBuffer,
        _resourceManager->meshManager().globalIndexBuffer(0),
        0,
        VK_INDEX_TYPE_UINT32
    );
    auto perDrawDescriptorSetHandler = _vkEngine.get().descriptorSetManager->getDescriptorSetHandler(
        CommonDescriptorSetName::PER_DRAW, drawContext.currentFrame
    );
    if (!perDrawDescriptorSetHandler.has_value()) {
        _logger.error("Failed to get per draw descriptor set handler!");
        return;
    }
    // TODO: fix this
    perDrawDescriptorSetHandler->bind(
        drawContext.commandBuffer.vkCommandBuffer, drawContext.pipelineLayout, MainSetIndices::PER_DRAW_SET_INDEX
    );
    if (auto drawRange = _indirectDrawController->drawRange("gizmo")) {
        GlobalPushConstants pushConstants{.drawIndexOffset = drawRange->start};
        vkCmdPushConstants(
            drawContext.commandBuffer.vkCommandBuffer,
            drawContext.pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(pushConstants),
            &pushConstants
        );
        _indirectDrawController->drawRange(drawContext.commandBuffer, drawContext.currentFrame, "gizmo");
    }
}

void Scene::onMouseMove(const vax::MouseMoveValue& value) {
    _mainCamera.rotateBy(value.delta);
    _gizmoCamera.rotateBy(value.delta);
}

void Scene::onMouseWheel(float delta) { _mainCamera.zoomBy(0.1f * delta); }

void Scene::onKeyEvent(const vax::KeyEvent& keyEvent) {}

void Scene::beginDrawing(CommandBuffer& commandBuffer, uint32_t frameIndex) {
    if (!_world) {
        _logger.warning("ecs world is not set!");
        return;
    }
    _indirectDrawController->prepareForDraw(frameIndex);

    uint32_t instanceCursor = 0;
    _indirectDrawController->addDrawScope("scene", [&]() {
        instanceCursor =
            _sceneComposer->compose(*_world, *_indirectDrawController, frameIndex, instanceCursor);
    });

    if (_descriptor->shouldDrawGizmo) {
        _indirectDrawController->addDrawScope("gizmo", [&]() {
            instanceCursor =
                _sceneComposer->compose(*_gizmoWorld, *_indirectDrawController, frameIndex, instanceCursor);
        });
    }

    _indirectDrawController->addDrawScope("background", [&]() {
        instanceCursor =
            _sceneComposer->compose(*_backgroundWorld, *_indirectDrawController, frameIndex, instanceCursor);
    });

    _indirectDrawController->submitCommands(frameIndex);
}

void Scene::endDrawing(CommandBuffer& commandBuffer, uint32_t frameIndex) {}