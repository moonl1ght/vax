#include "drawableScene.h"
#include "camera.h"
#include "gridWorldDescriptor.h"
#include "profiler.h"
#include "swapchain.h"

using namespace vax;
using namespace vax::vk;
using namespace vax::engine;
using namespace vax::rl;

void DrawableScene::prepareForDraw(engine::RenderCallContext renderCallContext) {
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

    if (auto mappedMemory = _roverCameraUniformBuffers[renderCallContext.currentFrame]->mappedMemory()) {
        memcpy(mappedMemory.value(), &_roverCameraUbo, sizeof(_roverCameraUbo));
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

void DrawableScene::update(engine::SceneUpdateContext sceneUpdateContext) {
    _sceneUpdateContext = sceneUpdateContext;
    _ubo = _mainCamera.getUniformBufferObject();
    _ubo.environmentMapIndex = 0;
    _drawableWorld->update(sceneUpdateContext.frameTime);

    auto& roverCamera = _drawableWorld->roverCamera();
    _roverCameraUbo = roverCamera.getUniformBufferObject();
    _roverCameraUbo.environmentMapIndex = 0;
    _sunLightUbo = _sunLight.camera().getUniformBufferObject();
    _sunLightUbo.environmentMapIndex = 0;
}

void vax::engine::DrawableScene::resize() {
    auto swapchainExtent = _vkEngine.get().getWindowController().getWindow(0)->getSwapchain()->swapchainExtent;
    _mainCamera.setViewPortSize(vax::math::SizeUI(swapchainExtent));
    _sunLight.camera().setViewPortSize(vax::math::SizeUI(swapchainExtent));
}

bool vax::engine::DrawableScene::writePerDrawDescriptorSet(vax::vk::DescriptorSetWriter& descriptorWriter) {
    _indirectDrawController->writePerDrawDescriptorSet(descriptorWriter, _renderCallContext.currentFrame);
    return true;
}

bool vax::engine::DrawableScene::writeGlobalDescriptorSet(vax::vk::DescriptorSetWriter& descriptorWriter) {
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

bool vax::engine::DrawableScene::writeFrameDescriptorSet(
    vax::vk::DescriptorSetWriter& descriptorWriter, vax::vk::DescriptorSetWriter& roverCameraDescriptorWriter
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

    roverCameraDescriptorWriter.writeBuffer(
        *_roverCameraUniformBuffers[_renderCallContext.currentFrame],
        PerFrameDescriptorSetResourceIndex::FRAME_UNIFORM_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    );
    roverCameraDescriptorWriter.writeBuffer(
        *_lightsUniformBuffer[_renderCallContext.currentFrame],
        PerFrameDescriptorSetResourceIndex::FRAME_LIGHT_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    );
    roverCameraDescriptorWriter.writeBuffer(
        _resourceManager->ssboManager().instanceBuffer(_renderCallContext.currentFrame),
        PerFrameDescriptorSetResourceIndex::FRAME_INSTANCE_BUFFER_INDEX,
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    );
    return true;
}

void vax::engine::DrawableScene::draw(const DrawContext& drawContext) {
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

void vax::engine::DrawableScene::drawBackground(const DrawContext& drawContext) {
    if (!_background)
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
    _indirectDrawController->drawRange(drawContext.commandBuffer, drawContext.currentFrame, "background");
}

void vax::engine::DrawableScene::drawGizmo(const DrawContext& drawContext) {
    // if (!_gizmo)
    //     return;
    // VkBuffer vertexBuffers[] = {_resourceManager.meshManager().globalVertexBuffer(0)};
    // VkDeviceSize offsets[] = {0};
    // vkCmdBindVertexBuffers(drawContext.commandBuffer.vkCommandBuffer, 0, 1, vertexBuffers, offsets);
    // vkCmdBindIndexBuffer(
    //     drawContext.commandBuffer.vkCommandBuffer,
    //     _resourceManager.meshManager().globalIndexBuffer(0),
    //     0,
    //     VK_INDEX_TYPE_UINT32
    // );
    // auto drawRange = _drawRanges[1];
    // auto perDrawDescriptorSetHandler = _vkEngine.get().descriptorSetManager->getDescriptorSetHandler(
    //     CommonDescriptorSetName::PER_DRAW, drawContext.currentFrame
    // );
    // if (!perDrawDescriptorSetHandler.has_value()) {
    //     _logger.error("Failed to get per draw descriptor set handler!");
    //     return;
    // }
    // // TODO: fix this
    // perDrawDescriptorSetHandler->bind(
    //     drawContext.commandBuffer.vkCommandBuffer, drawContext.pipelineLayout, MainSetIndices::PER_DRAW_SET_INDEX
    // );
    // GlobalPushConstants pushConstants{.drawIndexOffset = drawRange.start};
    // vkCmdPushConstants(
    //     drawContext.commandBuffer.vkCommandBuffer,
    //     drawContext.pipelineLayout,
    //     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    //     0,
    //     sizeof(pushConstants),
    //     &pushConstants
    // );
    // _indirectDrawController->drawRange(drawContext.commandBuffer, drawContext.currentFrame, drawRange);
}

void vax::engine::DrawableScene::onMouseMove(const vax::MouseMoveValue& value) {
    _mainCamera.rotateBy(value.delta);
    // _gizmoCamera.rotateBy(value.delta);
}

void vax::engine::DrawableScene::onMouseWheel(float delta) { _mainCamera.zoomBy(0.1f * delta); }

void vax::engine::DrawableScene::onKeyEvent(const vax::KeyEvent& keyEvent) {}

void DrawableScene::beginDrawing(CommandBuffer& commandBuffer, uint32_t frameIndex) {
    // auto viewMatrix = _gizmoCamera.viewMatrix();
    // auto projectionMatrix = _gizmoCamera.projectionMatrix();
    // auto viewProjectionMatrix = projectionMatrix * viewMatrix;
    // _gizmo->updateTransform([&](vax::math::TransformHandle& transformHandle) {
    //     transformHandle.setCachedTransformMatrix(viewProjectionMatrix);
    // });
    _indirectDrawController->prepareForDraw(frameIndex);

    _indirectDrawController->addDrawScope("scene", [&]() {
        _drawableWorld->prepareDrawing(_indirectDrawController.get(), frameIndex);
    });

    // auto gizmoDrawRange = _indirectDrawController->addDrawScope([&]() {
    //     _gizmo->prepareDrawing(_indirectDrawController.get(), frameIndex);
    // });

    _indirectDrawController->addDrawScope("background", [&]() {
        _background->prepareDrawing(_indirectDrawController.get(), frameIndex);
    });

    _indirectDrawController->submitCommands(frameIndex);
}

void DrawableScene::endDrawing(CommandBuffer& commandBuffer, uint32_t frameIndex) {}