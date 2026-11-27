#include "editorRenderer.h"
#include "paths.h"
#include "renderPassGraphBuilder.h"
#include "renderTarget.h"
#include "renderTargetBuilder.h"
#include "sceneDrawData.h"

using namespace vax;
using namespace vax::gfx;

bool EditorRenderer::setup() noexcept {
    if (!_buildRenderTarget()) [[unlikely]] {
        return false;
    }
    if (!_buildPipelines()) [[unlikely]] {
        return false;
    }
    _renderPassGraphBuilder = std::make_unique<RenderPassGraphBuilder>(
        *_vkEngine.get().device,
        _windowController.get(),
        *_vkEngine.get().pipelineManager,
        _uiEngine.get(),
        _resourceManager.get()
    );
    _renderPassGraphBuilder->prebuild(*_renderTarget);
    if (!_buildRenderPassGraph()) [[unlikely]] {
        return false;
    }
    return true;
}

bool EditorRenderer::_buildRenderPassGraph() noexcept {
    _uiRenderPassGraph = _renderPassGraphBuilder->buildUIOnlyGraph(*_renderTarget);
    if (!_uiRenderPassGraph) [[unlikely]] {
        _logger.error("UI render pass graph not set!");
        return false;
    }
    _sceneRenderPassGraph = _renderPassGraphBuilder->buildEditorSceneGraph(*_renderTarget);
    if (!_sceneRenderPassGraph) [[unlikely]] {
        _logger.error("Scene render pass graph not set!");
        return false;
    }
    return true;
}

bool EditorRenderer::_buildRenderTarget() noexcept {
    auto renderTargetBuilder = RenderTargetBuilder(
        *_vkEngine.get().device,
        _vkEngine.get().pipelineManager->descriptorRegistry(),
        *_vkEngine.get().commandManager,
        *_vkEngine.get().queueManager
    );
    _renderTarget = renderTargetBuilder.buildForEditor(*_windowController.get().getWindow(0)->vkWindow()->swapchain());
    if (!_renderTarget) [[unlikely]] {
        _logger.error("Failed to build render target!");
        return false;
    }
    return true;
}

bool EditorRenderer::_buildPipelines() noexcept {
    auto& pipelineManager = *_vkEngine.get().pipelineManager;
    if (!pipelineManager.load(SRC_PATH("gfx/shaders/"))) [[unlikely]] {
        _logger.error("Failed to load pipeline manager!");
        return false;
    }

    auto* swapchainDescriptor = _renderTarget->getRenderPassDescriptor("swapchain");
    auto* mainDescriptor = _renderTarget->getRenderPassDescriptor("main");
    auto* shadowSunDescriptor = _renderTarget->getRenderPassDescriptor("shadow_sun");
    if (!swapchainDescriptor || !mainDescriptor || !shadowSunDescriptor) [[unlikely]] {
        _logger.error("Render target is missing render pass descriptors!");
        return false;
    }

    auto defaultPipelines = vax::rhi::defaultPipelines();
    defaultPipelines["finalblend"].renderPassDescriptor = swapchainDescriptor;
    defaultPipelines["finalblend_cam"].renderPassDescriptor = swapchainDescriptor;
    defaultPipelines["pbr"].renderPassDescriptor = mainDescriptor;
    defaultPipelines["simple"].renderPassDescriptor = mainDescriptor;
    defaultPipelines["shadow"].renderPassDescriptor = shadowSunDescriptor;

    std::vector<vax::rhi::PipelineDescriptor> pipelineDescriptors;
    pipelineDescriptors.reserve(defaultPipelines.size());
    for (const auto& [name, pipelineDescriptor] : defaultPipelines) {
        pipelineDescriptors.push_back(pipelineDescriptor);
    }
    if (!pipelineManager.prebuildPipelines(pipelineDescriptors)) [[unlikely]] {
        _logger.error("Failed to prebuild pipelines!");
        return false;
    }
    return true;
}

bool EditorRenderer::render(scene::EditorScene* scene, const time::FrameTime& frameTime) {
    ZoneScopedN("Renderer::render");
    _waitForFence();

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(
        _vkEngine.get().device->vkDevice(),
        _windowController.get().getWindow(0)->vkWindow()->swapchain()->swapchain,
        UINT64_MAX,
        _vkEngine.get().syncObjectsManager->getImageAvailableSemaphores("main")[_currentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) [[unlikely]] {
        _resize(scene);
        return false;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) [[unlikely]] {
        _logger.error("Failed to acquire swap chain image!");
        return false;
    }

    if (_frameProfiler) {
        _frameProfiler->beginFrameZone("frame_workload");
    }

    vkResetFences(
        _vkEngine.get().device->vkDevice(), 1, &_vkEngine.get().syncObjectsManager->getInFlightFences()[_currentFrame]
    );

    auto& commandBuffer = _vkEngine.get().commandManager->getCommandBuffer(_currentFrame);
    commandBuffer.reset();

    auto updateResult = _updateCommandBuffer(commandBuffer, imageIndex, scene);
    if (!updateResult) {
        _logger.error("Failed to update command buffer!");
        return false;
    }

    auto submitResult = _submit(commandBuffer);
    if (!submitResult) {
        _logger.error("Failed to submit command buffer!");
        return false;
    }

    auto presentResult = _present(scene, imageIndex);
    if (!presentResult) {
        _logger.error("Failed to present image!");
        return false;
    }

    _currentFrame = (_currentFrame + 1) % vax::rhi::MAX_FRAMES_IN_FLIGHT;

    if (_frameProfiler) {
        _frameProfiler->endFrameZone("frame_workload");
    }
    return true;
}

bool EditorRenderer::_updateCommandBuffer(
    vax::rhi::CommandBuffer& commandBuffer, uint32_t imageIndex, scene::EditorScene* scene
) {
    ZoneScopedN("Renderer::updateCommandBuffer");
    if (!commandBuffer.begin())
        return false;

    if (scene != nullptr) {
        _drawScene(commandBuffer, scene, imageIndex);
    } else {
        _drawUI(commandBuffer, imageIndex);
    }

    if (!commandBuffer.end())
        return false;

    return true;
}

void EditorRenderer::_drawUI(vax::rhi::CommandBuffer& commandBuffer, uint32_t imageIndex) noexcept {
    RenderPassNode::RunPassInfo runPassInfo{
        .commandBuffer = commandBuffer,
        .scene = nullptr,
        .imageIndices = {imageIndex},
        .frameIndex = _currentFrame,
    };

    if (!_uiRenderPassGraph) [[unlikely]] {
        _logger.error("UI render pass graph not set!");
        return;
    }

    _uiRenderPassGraph->run(runPassInfo);
}

void EditorRenderer::_drawScene(
    vax::rhi::CommandBuffer& commandBuffer, scene::EditorScene* scene, uint32_t imageIndex
) noexcept {
    ZoneScopedN("Renderer::_drawScene");
    if (!_bindBindlessDescriptorSet(commandBuffer)) [[unlikely]] {
        _logger.error("Failed to bind bindless descriptor set!");
        return;
    }

    RenderPassNode::FrameTextureHandles frameTextures{};
    auto mainRenderDestination = _renderTarget->getRenderDestination("main");
    if (_currentFrame < mainRenderDestination->colorTextures().size()) {
        frameTextures.mainColor = mainRenderDestination->colorTextures()[_currentFrame]->sampledShaderHandle();
        frameTextures.mainMask = mainRenderDestination->maskTextures()[_currentFrame]->sampledShaderHandle();
        frameTextures.mainDepth = mainRenderDestination->depthTexture()->sampledShaderHandle();
    }

    gfx::SceneDrawData::VkContext vkContext{
        .indexBuffer = _resourceManager.get().meshManager().globalIndexBuffer(0).vkBuffer(),
        .pipelineLayout = _vkEngine.get().pipelineManager->pipelineLayout(),
        .uboDescriptorSet = _vkEngine.get().pipelineManager->uboDescriptorSet(_currentFrame),
        .frameUniformStride = _uniformBufferStride,
    };

    gfx::SceneDrawData::Handles handles{
        .vertices = _resourceManager.get().meshManager().globalVertexBuffer(0).first,
        .instances = _resourceManager.get().ssboManager().instanceBuffer(_currentFrame).first,
        .drawData = _indirectDrawController->perDrawDataHandle(_currentFrame),
        .materials = _resourceManager.get().materialManager().materialBuffer().first,
    };

    gfx::SceneDrawData sceneDrawData{*_indirectDrawController, handles, vkContext};

    RenderPassNode::RunPassInfo runPassInfo{
        .commandBuffer = commandBuffer,
        .scene = nullptr,
        .imageIndices = {imageIndex},
        .frameIndex = _currentFrame,
        .frameTextures = frameTextures,
        .sceneDrawData = std::make_optional(sceneDrawData),
    };

    _prepareDrawRanges(scene);
    _sceneRenderPassGraph->run(runPassInfo);
}

void EditorRenderer::_resize(scene::EditorScene* scene) noexcept {
    _windowController.get().resize();
    _vkEngine.get().device->waitIdle();
    _windowController.get().getWindow(0)->vkWindow()->swapchain()->recreate();
    if (scene != nullptr) {
        scene->resize();
    }
    if (!_buildRenderTarget()) [[unlikely]] {
        _logger.error("Failed to build render target!");
        return;
    }
    if (!_buildRenderPassGraph()) [[unlikely]] {
        _logger.error("Failed to build render pass graph!");
        return;
    }
}

bool EditorRenderer::_submit(vax::rhi::CommandBuffer& commandBuffer) noexcept {
    auto& sync = *_vkEngine.get().syncObjectsManager;

    const VkSemaphoreSubmitInfo waitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = sync.getImageAvailableSemaphores("main")[_currentFrame],
        .value = 0,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .deviceIndex = 0,
    };

    const VkSemaphoreSubmitInfo signalInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = sync.getRenderFinishedSemaphores("main")[_currentFrame],
        .value = 0,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .deviceIndex = 0,
    };

    const VkCommandBufferSubmitInfo cmdInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = commandBuffer.vkCommandBuffer,
        .deviceMask = 0,
    };

    const VkSubmitInfo2 submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &waitInfo,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdInfo,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signalInfo,
    };

    return _vkEngine.get().queueManager->getMainGraphicsQueue().submit(
        submitInfo, sync.getInFlightFences()[_currentFrame]
    );
}

bool EditorRenderer::_present(scene::EditorScene* scene, uint32_t imageIndex) noexcept {
    std::vector<VkSemaphore> signalSemaphores;
    signalSemaphores.push_back(_vkEngine.get().syncObjectsManager->getRenderFinishedSemaphores("main")[_currentFrame]);
    std::vector<VkSwapchainKHR> swapchains;
    swapchains.push_back(_windowController.get().getWindow(0)->vkWindow()->swapchain()->swapchain);
    std::vector<uint32_t> imageIndices;
    imageIndices.push_back(imageIndex);

    std::vector<VkResult> presentResults(swapchains.size(), VK_SUCCESS);
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = static_cast<uint32_t>(swapchains.size()),
        .pSwapchains = swapchains.data(),
        .pImageIndices = imageIndices.data(),
        .pResults = presentResults.data(),
    };

    vkQueuePresentKHR(_vkEngine.get().queueManager->getMainPresentQueue().vkQueue(), &presentInfo);
    auto isStale = [](VkResult presentResult) {
        return presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR;
    };
    bool isMainStale = isStale(presentResults[0]);
    bool hasPresentError = false;
    for (VkResult presentResult : presentResults) {
        hasPresentError |= presentResult != VK_SUCCESS && !isStale(presentResult);
    }

    if (isMainStale) {
        _resize(scene);
        return false;
    }
    if (hasPresentError) {
        return false;
    }
    return true;
}

bool EditorRenderer::_prepareDrawRanges(scene::EditorScene* scene) noexcept {
    _indirectDrawController->prepareForDraw(_currentFrame);

    uint32_t instanceCursor = 0;
    auto& gizmoWorld = scene->gizmo().world();
    _indirectDrawController->addDrawRange("gizmo", [&]() {
        instanceCursor = _sceneComposer->compose(gizmoWorld, *_indirectDrawController, _currentFrame, instanceCursor);
    });

    _indirectDrawController->submitCommands(_currentFrame);
    return true;
}