#include "editorRenderer_old.h"
#include "imageUtils.h"
#include "paths.h"
#include "profiler.h"
#include "vkEngine.h"
#include <array>

using namespace vax::gfx;
using namespace vax;
using namespace vax::rhi;

void EditorRenderer_old::setup(vax::rhi::ResourceManager& resourceManager) noexcept {
    _renderPassGraphManager = std::make_unique<RenderPassGraphManager>(
        *_vkEngine.get().device,
        *_vkEngine.get().pipelineManager,
        resourceManager,
        _windowController.get(),
        _uiEngine.get()
    );
    auto swapchain = _getSwapchain(0);
    _renderPassGraphManager->setupRenderPassDescriptors(swapchain->swapchainImageFormat);
    auto swapchainRenderPassDescriptor = _renderPassGraphManager->getRenderPassDescriptor("swapchain");
    _uiEngine.get().setup(swapchainRenderPassDescriptor->getVkRenderPass());

    _renderPassGraphManager->setupRenderDestinations(
        *_vkEngine.get().commandManager, *_vkEngine.get().queueManager, *swapchain
    );
}

void EditorRenderer_old::load() {
    if (_isLoaded) {
        return;
    }
    _vkEngine.get().pipelineManager->load(SRC_PATH("gfx/shaders/"));

    auto swapchain = _getSwapchain(0);
    auto swapchainRenderPassDescriptor = _renderPassGraphManager->getRenderPassDescriptor("swapchain");
    auto mainRenderPassDescriptor = _renderPassGraphManager->getRenderPassDescriptor("main");
    auto shadowSunRenderPassDescriptor = _renderPassGraphManager->getRenderPassDescriptor("shadow_sun");

    auto defaultPipelines = vax::rhi::defaultPipelines();
    defaultPipelines["finalblend"].renderPassDescriptor = swapchainRenderPassDescriptor;
    defaultPipelines["finalblend_cam"].renderPassDescriptor = swapchainRenderPassDescriptor;
    defaultPipelines["pbr"].renderPassDescriptor = mainRenderPassDescriptor;
    defaultPipelines["simple"].renderPassDescriptor = mainRenderPassDescriptor;
    defaultPipelines["shadow"].renderPassDescriptor = shadowSunRenderPassDescriptor;
    std::vector<vax::rhi::PipelineDescriptor> pipelineDescriptors;
    for (const auto& [name, pipelineDescriptor] : defaultPipelines) {
        pipelineDescriptors.push_back(pipelineDescriptor);
    }
    _vkEngine.get().pipelineManager->prebuildPipelines(pipelineDescriptors);

    _isLoaded = true;
}

void EditorRenderer_old::_prepareForRender(Scene* scene) {
    ZoneScopedN("Renderer::prepareForRender");
    bool shouldRebuildGraph = renderPassGraphMode.is_dirty() || _wasResized;
    if (scene != nullptr) {
        auto& shadowMapResource = _renderPassGraphManager->getRenderDestination("shadow_sun")->depthTexture();
        scene->prepareForDraw(
            gfx::RenderCallContext{
            .currentFrame = _currentFrame,
            .shadowMapIndex =
                shadowMapResource ? shadowMapResource->sampledShaderHandle().index : NullShaderResourceIndex,
            }
        );

        shouldRebuildGraph |= scene->shouldRebuildRenderPassGraph();
    }

    if (_wasResized) {
        _wasResized = false;
    }

    if (shouldRebuildGraph) {
        switch (renderPassGraphMode.get_and_clear_dirty()) {
        case RenderPassGraphMode::UI_ONLY:
            _renderPassGraphManager->buildGraph(RenderPassGraphMode::UI_ONLY, nullptr);
            break;
        case RenderPassGraphMode::SCENE_GRAPH:
            if (scene != nullptr) {
                if (scene->renderPassGraphDescriptor().withSecondaryWindow) {
                    _renderPassGraphManager->setupRenderDestinationsForAgentCamera(
                        *_vkEngine.get().commandManager, *_vkEngine.get().queueManager, *_getSwapchain(1)
                    );
                }
                _renderPassGraphManager->buildGraph(
                    RenderPassGraphMode::SCENE_GRAPH, scene != nullptr ? &scene->renderPassGraphDescriptor() : nullptr
                );
            }
            break;
        case RenderPassGraphMode::UNKNOWN:
            _logger.error("Unknown render pass graph mode!");
            break;
        }
    }
}

bool EditorRenderer_old::render(Scene* scene, const time::FrameTime& frameTime) {
    _prepareForRender(scene);
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
    bool drawSecondaryWindow = false;
    if (scene != nullptr) {
        drawSecondaryWindow = scene->renderPassGraphDescriptor().withSecondaryWindow;
    }
    uint32_t agentCameraImageIndex = UINT32_MAX;
    if (drawSecondaryWindow) {
        VkResult agentCameraResult = vkAcquireNextImageKHR(
            _vkEngine.get().device->vkDevice(),
            _windowController.get().getWindow(1)->vkWindow()->swapchain()->swapchain,
            UINT64_MAX,
            _vkEngine.get().syncObjectsManager->getImageAvailableSemaphores("secondary")[_currentFrame],
            VK_NULL_HANDLE,
            &agentCameraImageIndex
        );
        if (agentCameraResult == VK_ERROR_OUT_OF_DATE_KHR) {
            _resizeSecondary();
            drawSecondaryWindow = false;
            agentCameraImageIndex = UINT32_MAX;
        } else if (agentCameraResult != VK_SUCCESS && agentCameraResult != VK_SUBOPTIMAL_KHR) {
            _logger.warning("Failed to acquire agent camera swap chain image, skipping secondary window this frame");
            drawSecondaryWindow = false;
            agentCameraImageIndex = UINT32_MAX;
        }
    }

    _outputImageIndices = {imageIndex, agentCameraImageIndex};

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        _vkEngine.get().resize();
        _resize();
        if (scene != nullptr) {
            scene->resize();
        }
        return false;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
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

    auto updateResult = _updateCommandBuffer(commandBuffer, imageIndex, agentCameraImageIndex, scene);
    if (!updateResult) {
        _logger.error("Failed to update command buffer!");
        return false;
    }

    std::vector<VkSemaphore> waitSemaphores;
    waitSemaphores.push_back(_vkEngine.get().syncObjectsManager->getImageAvailableSemaphores("main")[_currentFrame]);
    if (drawSecondaryWindow) {
        waitSemaphores.push_back(
            _vkEngine.get().syncObjectsManager->getImageAvailableSemaphores("secondary")[_currentFrame]
        );
    }
    std::vector<VkPipelineStageFlags> waitStages(waitSemaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    std::vector<VkSemaphore> signalSemaphores;
    signalSemaphores.push_back(_vkEngine.get().syncObjectsManager->getRenderFinishedSemaphores("main")[_currentFrame]);
    if (drawSecondaryWindow) {
        signalSemaphores.push_back(
            _vkEngine.get().syncObjectsManager->getRenderFinishedSemaphores("secondary")[_currentFrame]
        );
    }
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer.vkCommandBuffer,
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pSignalSemaphores = signalSemaphores.data()
    };

    auto submitResult = _vkEngine.get().queueManager->getMainGraphicsQueue().submitV1(
        submitInfo, _vkEngine.get().syncObjectsManager->getInFlightFences()[_currentFrame]
    );
    if (!submitResult) {
        _logger.error("failed to submit draw command buffer!");
        return false;
    }

    std::vector<VkSwapchainKHR> swapChains;
    swapChains.push_back(_getSwapchain(0)->swapchain);
    std::vector<uint32_t> imageIndices;
    imageIndices.push_back(imageIndex);
    if (drawSecondaryWindow) {
        swapChains.push_back(_getSwapchain(1)->swapchain);
        imageIndices.push_back(agentCameraImageIndex);
    }
    std::vector<VkResult> presentResults(swapChains.size(), VK_SUCCESS);
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = static_cast<uint32_t>(swapChains.size()),
        .pSwapchains = swapChains.data(),
        .pImageIndices = imageIndices.data(),
        .pResults = presentResults.data(),
    };

    result = vkQueuePresentKHR(_vkEngine.get().queueManager->getMainPresentQueue().vkQueue(), &presentInfo);
    auto isStale = [](VkResult presentResult) {
        return presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR;
    };
    bool isMainStale = isStale(presentResults[0]) || _vkEngine.get().framebufferResized;
    bool isSecondaryStale = drawSecondaryWindow && isStale(presentResults[1]);
    bool hasPresentError = false;
    for (VkResult presentResult : presentResults) {
        hasPresentError |= presentResult != VK_SUCCESS && !isStale(presentResult);
    }

    if (isSecondaryStale) {
        _resizeSecondary();
    }
    if (isMainStale) {
        _vkEngine.get().framebufferResized = false;
        _vkEngine.get().resize();
        _resize();
        if (scene != nullptr) {
            scene->resize();
        }
        return false;
    }
    if (hasPresentError) {
        _logger.error("failed to present swap chain image!");
        return false;
    }

    _currentFrame = (_currentFrame + 1) % vax::rhi::MAX_FRAMES_IN_FLIGHT;

    if (_frameProfiler) {
        _frameProfiler->endFrameZone("frame_workload");
    }
    return true;
}

bool EditorRenderer_old::_updateCommandBuffer(
    CommandBuffer& commandBuffer, uint32_t imageIndex, uint32_t agentCameraImageIndex, vax::gfx::Scene* scene
) {
    ZoneScopedN("Renderer::updateCommandBuffer");
    if (!commandBuffer.begin())
        return false;

    if (scene != nullptr) {
        _drawScene(commandBuffer, scene);
    } else {
        _drawUi(commandBuffer);
    }

    if (!commandBuffer.end())
        return false;
    return true;
}

void EditorRenderer_old::_drawUi(CommandBuffer& commandBuffer) {
    RenderPassNode::RunPassInfo runPassInfo{
        .commandBuffer = commandBuffer,
        .scene = nullptr,
        .imageIndices = _outputImageIndices,
        .frameIndex = _currentFrame,
    };

    _renderPassGraphManager->getRenderPassGraph().run(runPassInfo);
}

bool EditorRenderer_old::_drawScene(CommandBuffer& commandBuffer, Scene* scene) {
    ZoneScopedN("Renderer::_drawScene");
    if (!_bindDescriptorSets(commandBuffer))
        return false;

    RenderPassNode::FrameTextureHandles frameTextures{};
    auto mainRenderDestination = _renderPassGraphManager->getRenderDestination("main");
    bool drawSecondaryWindow = scene->renderPassGraphDescriptor().withSecondaryWindow;
    if (_currentFrame < mainRenderDestination->colorTextures().size()) {
        frameTextures.mainColor = mainRenderDestination->colorTextures()[_currentFrame]->sampledShaderHandle();
        frameTextures.mainMask = mainRenderDestination->maskTextures()[_currentFrame]->sampledShaderHandle();
        frameTextures.mainDepth = mainRenderDestination->depthTexture()->sampledShaderHandle();
    }
    if (drawSecondaryWindow) {
        auto agentCameraRenderDestination = _renderPassGraphManager->getRenderDestination("agent_camera_main");
        if (_currentFrame < agentCameraRenderDestination->colorTextures().size()) {
            frameTextures.agentCameraColor =
                agentCameraRenderDestination->colorTextures()[_currentFrame]->sampledShaderHandle();
        }
    }

    RenderPassNode::RunPassInfo runPassInfo{
        .commandBuffer = commandBuffer,
        .scene = scene,
        .imageIndices = _outputImageIndices,
        .frameIndex = _currentFrame,
        .frameTextures = frameTextures,
    };

    scene->beginDrawing(commandBuffer, _currentFrame);
    _renderPassGraphManager->getRenderPassGraph().run(runPassInfo);
    return true;
}

bool EditorRenderer_old::_bindDescriptorSets(CommandBuffer& commandBuffer) {
    auto& pipelineManager = *_vkEngine.get().pipelineManager;
    VkPipelineLayout pipelineLayout = pipelineManager.pipelineLayout();
    std::array<VkDescriptorSet, 2> descriptorSets{
        pipelineManager.bindlessDescriptorSet(),
        pipelineManager.uboDescriptorSet(_currentFrame),
    };
    if (pipelineLayout == VK_NULL_HANDLE || descriptorSets[BINDLESS_SET_INDEX] == VK_NULL_HANDLE ||
        descriptorSets[UBO_SET_INDEX] == VK_NULL_HANDLE) {
        _logger.error("Pipeline layout or descriptor sets are not created!");
        return false;
    }
    uint32_t frameUniformBufferOffset = 0;
    vkCmdBindDescriptorSets(
        commandBuffer.vkCommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        BINDLESS_SET_INDEX,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(),
        1,
        &frameUniformBufferOffset
    );
    return true;
}

void EditorRenderer_old::_resize() {
    auto swapchain = _getSwapchain(0);
    _renderPassGraphManager->setupRenderDestinations(
        *_vkEngine.get().commandManager, *_vkEngine.get().queueManager, *swapchain
    );
    _wasResized = true;
}

void EditorRenderer_old::_resizeSecondary() {
    vkDeviceWaitIdle(_vkEngine.get().device->vkDevice());
    _getSwapchain(1)->recreate();
    _wasResized = true;
}

vax::rhi::Swapchain* EditorRenderer_old::_getSwapchain(size_t index) {
    if (index >= editor::WindowController::maxWindows) {
        _logger.error("Invalid window index");
        return nullptr;
    }
    return _windowController.get().getWindow(index)->vkWindow()->swapchain();
}