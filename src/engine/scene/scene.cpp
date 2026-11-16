#include "scene.h"
#include "base.h"
#include "camera.h"
#include "components.h"
#include "pbr.h"
#include "profiler.h"
#include "swapchain.h"

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
    _lightsUniformBuffer.reserve(vax::vk::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto& bufferManager = _resourceManager->bufferManager();
        auto allocation = bufferManager
                              .allocateBuffer(
                                  "frame_uniform_buffer",
                                  _frameUniformStride() * static_cast<uint32_t>(FrameUniformSlot::SLOTS_COUNT),
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  VMA_MEMORY_USAGE_CPU_TO_GPU,
                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                              )
                              .value();
        allocation.second->map();
        _sceneUniformBuffers.push_back(allocation.second);
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

    _indirectDrawController->setup(10000, _vkEngine.get().pipelineManager->descriptorRegistry());

    auto loadQueue = _vkEngine.get().queueManager->graphicsQueue;
    auto loadCommandBuffer = _vkEngine.get().commandManager->createSingleTimeCommandBuffer();

    loadCommandBuffer.begin();

    _resourceManager->textureManager().loadAllTextures(loadCommandBuffer);

    loadCommandBuffer.end();
    loadCommandBuffer.submitAndWait(loadQueue);

    _resourceManager->textureManager().resetAllTexturesStagingBuffers();

    registerResources();

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
        auto stride = _frameUniformStride();
        auto slotOffset = [stride](FrameUniformSlot slot) { return static_cast<uint32_t>(slot) * stride; };
        memcpy(mappedPtr + slotOffset(FrameUniformSlot::MAIN_CAMERA), &_ubo, sizeof(_ubo));
        memcpy(mappedPtr + slotOffset(FrameUniformSlot::SUN_LIGHT), &_sunLightUbo, sizeof(_sunLightUbo));
        memcpy(mappedPtr + slotOffset(FrameUniformSlot::AGENT_CAMERA), &_agentCameraUbo, sizeof(_agentCameraUbo));
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

void Scene::registerResources() noexcept {
    auto& registry = _vkEngine.get().pipelineManager->descriptorRegistry();
    auto& textureManager = _resourceManager->textureManager();

    if (_resourcesRegistered) {
        _releaseResources();
    }

    for (auto globalSampler : {GlobalSampler::PBRSampler, GlobalSampler::CubeMapSampler}) {
        auto sampler = textureManager.getGlobalSampler(globalSampler);
        if (!sampler) {
            _logger.error("Global sampler is missing!");
            return;
        }
        auto handle = registry.registerSampler(sampler->second->vkSampler);
        if (!handle) {
            _logger.error("Failed to register sampler!");
            return;
        }
        if (handle->index != static_cast<uint32_t>(globalSampler)) {
            _logger.warning(
                "Global sampler ", static_cast<uint32_t>(globalSampler), " registered at index ", handle->index
            );
        }
        _samplerHandles.push_back(*handle);
    }

    auto textures = textureManager.allTextures();
    if (!textures.empty()) {
        auto handles = registry.registerSampledTexture(textures, /*contiguous*/ true);
        if (!handles) {
            _logger.error("Failed to register scene textures!");
            return;
        }
        _textureHandles = std::move(*handles);
        if (_textureHandles.front().index != 0) {
            _logger.warning(
                "Scene textures don't start at index 0, material texture indices are off by ",
                _textureHandles.front().index
            );
        }
    }

    auto vertices = registry.registerStorageBuffer(_resourceManager->meshManager().globalVertexBuffer(0));
    auto materials = registry.registerStorageBuffer(_resourceManager->materialManager().materialBuffer());
    auto environmentMaps = registry.registerStorageBuffer(_environmentMap->environmentMapBuffer());
    if (!vertices || !materials || !environmentMaps) {
        _logger.error("Failed to register scene storage buffers!");
        return;
    }
    _vertexBufferHandle = *vertices;
    _materialBufferHandle = *materials;
    _environmentMapBufferHandle = *environmentMaps;

    for (uint32_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto instances = registry.registerStorageBuffer(_resourceManager->ssboManager().instanceBuffer(i));
        if (!instances) {
            _logger.error("Failed to register instance buffer ", i);
            return;
        }
        _instanceBufferHandles[i] = *instances;
    }

    _writeUniformDescriptorSets();

    _resourcesRegistered = true;
}

void Scene::_writeUniformDescriptorSets() noexcept {
    auto& pipelineManager = *_vkEngine.get().pipelineManager;
    for (uint32_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo frameBufferInfo{
            .buffer = _sceneUniformBuffers[i]->vkBuffer(),
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };
        VkDescriptorBufferInfo lightBufferInfo{
            .buffer = _lightsUniformBuffer[i]->vkBuffer(),
            .offset = 0,
            .range = sizeof(LightUBO),
        };
        VkDescriptorSet uboDescriptorSet = pipelineManager.uboDescriptorSet(i);
        std::array<VkWriteDescriptorSet, 2> writes{
            VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = uboDescriptorSet,
            .dstBinding = FRAME_UNIFORM_BUFFER_INDEX,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .pBufferInfo = &frameBufferInfo,
            },
            VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = uboDescriptorSet,
            .dstBinding = LIGHT_UNIFORM_BUFFER_INDEX,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &lightBufferInfo,
            },
        };
        vkUpdateDescriptorSets(
            _vkEngine.get().device->vkDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr
        );
    }
}

uint32_t Scene::_frameUniformStride() const noexcept {
    return static_cast<uint32_t>(_vkEngine.get().device->minUniformBufferOffsetAlignment<UniformBufferObject>());
}

void Scene::_releaseResources() noexcept {
    if (!_resourcesRegistered) {
        return;
    }
    auto& registry = _vkEngine.get().pipelineManager->descriptorRegistry();
    for (auto handle : _samplerHandles) {
        registry.releaseSampler(handle);
    }
    for (auto handle : _textureHandles) {
        registry.releaseSampledTexture(handle);
    }
    registry.releaseStorageBuffer(_vertexBufferHandle);
    registry.releaseStorageBuffer(_materialBufferHandle);
    registry.releaseStorageBuffer(_environmentMapBufferHandle);
    for (auto handle : _instanceBufferHandles) {
        registry.releaseStorageBuffer(handle);
    }
    _samplerHandles.clear();
    _textureHandles.clear();
    _resourcesRegistered = false;
}

void Scene::_bindIndexBuffer(const DrawContext& drawContext) const noexcept {
    vkCmdBindIndexBuffer(
        drawContext.commandBuffer.vkCommandBuffer,
        _resourceManager->meshManager().globalIndexBuffer(0).vkBuffer(),
        0,
        VK_INDEX_TYPE_UINT32
    );
}

void Scene::_bindFrameUniformSlot(const DrawContext& drawContext, FrameUniformSlot slot) const noexcept {
    auto& pipelineManager = *_vkEngine.get().pipelineManager;
    VkDescriptorSet uboDescriptorSet = pipelineManager.uboDescriptorSet(drawContext.currentFrame);
    uint32_t dynamicOffset = static_cast<uint32_t>(slot) * _frameUniformStride();
    vkCmdBindDescriptorSets(
        drawContext.commandBuffer.vkCommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineManager.pipelineLayout(),
        UBO_SET_INDEX,
        1,
        &uboDescriptorSet,
        1,
        &dynamicOffset
    );
}

void Scene::draw(const DrawContext& drawContext, FrameUniformSlot cameraSlot) {
    auto drawRange = _indirectDrawController->drawRange("scene");
    if (!drawRange) {
        return;
    }
    _bindIndexBuffer(drawContext);
    _bindFrameUniformSlot(drawContext, cameraSlot);

    PBRPushConstants pushConstants{
        .vertices = _vertexBufferHandle,
        .instances = _instanceBufferHandles[drawContext.currentFrame],
        .drawData = _indirectDrawController->perDrawDataHandle(drawContext.currentFrame),
        .materials = _materialBufferHandle,
        .environmentMaps = _environmentMapBufferHandle,
        .drawIndexOffset = drawRange->start,
    };
    vkCmdPushConstants(
        drawContext.commandBuffer.vkCommandBuffer,
        _vkEngine.get().pipelineManager->pipelineLayout(),
        VK_SHADER_STAGE_ALL,
        0,
        sizeof(pushConstants),
        &pushConstants
    );
    _indirectDrawController->drawRange(drawContext.commandBuffer, drawContext.currentFrame, "scene");
}

void Scene::drawShadow(const DrawContext& drawContext) {
    auto drawRange = _indirectDrawController->drawRange("scene");
    if (!drawRange) {
        return;
    }
    _bindIndexBuffer(drawContext);
    _bindFrameUniformSlot(drawContext, FrameUniformSlot::SUN_LIGHT);

    BasePushConstants pushConstants{
        .vertices = _vertexBufferHandle,
        .instances = _instanceBufferHandles[drawContext.currentFrame],
        .drawData = _indirectDrawController->perDrawDataHandle(drawContext.currentFrame),
        .drawIndexOffset = drawRange->start,
    };
    vkCmdPushConstants(
        drawContext.commandBuffer.vkCommandBuffer,
        _vkEngine.get().pipelineManager->pipelineLayout(),
        VK_SHADER_STAGE_ALL,
        0,
        sizeof(pushConstants),
        &pushConstants
    );
    _indirectDrawController->drawRange(drawContext.commandBuffer, drawContext.currentFrame, "scene");
}

void Scene::drawBackground(const DrawContext& drawContext) {
    vkCmdDraw(drawContext.commandBuffer.vkCommandBuffer, 3, 1, 0, 0);
}

void Scene::drawGizmo(const DrawContext& drawContext) {
    if (!_descriptor->shouldDrawGizmo)
        return;
    auto drawRange = _indirectDrawController->drawRange("gizmo");
    if (!drawRange) {
        return;
    }
    _bindIndexBuffer(drawContext);

    BasePushConstants pushConstants{
        .vertices = _vertexBufferHandle,
        .instances = _instanceBufferHandles[drawContext.currentFrame],
        .drawData = _indirectDrawController->perDrawDataHandle(drawContext.currentFrame),
        .drawIndexOffset = drawRange->start,
    };
    vkCmdPushConstants(
        drawContext.commandBuffer.vkCommandBuffer,
        _vkEngine.get().pipelineManager->pipelineLayout(),
        VK_SHADER_STAGE_ALL,
        0,
        sizeof(pushConstants),
        &pushConstants
    );
    _indirectDrawController->drawRange(drawContext.commandBuffer, drawContext.currentFrame, "gizmo");
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
        instanceCursor = _sceneComposer->compose(*_world, *_indirectDrawController, frameIndex, instanceCursor);
    });

    if (_descriptor->shouldDrawGizmo) {
        _indirectDrawController->addDrawScope("gizmo", [&]() {
            instanceCursor =
                _sceneComposer->compose(*_gizmoWorld, *_indirectDrawController, frameIndex, instanceCursor);
        });
    }

    _indirectDrawController->submitCommands(frameIndex);
}

void Scene::endDrawing(CommandBuffer& commandBuffer, uint32_t frameIndex) {}