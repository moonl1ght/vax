#include "renderer.h"
#include "descriptorSetManager.h"
#include "imageUtils.h"
#include "jfaPass.h"
#include "pipeline.h"
#include "profiler.h"
#include "textureFactory.h"
#include "vkEngine.h"

using namespace vax::engine;
using namespace vax;
using namespace vax::vk;

void Renderer::load() {
    auto swapchain = _getSwapchain(0);
    _renderPassGraphManager->setupRenderPassDescriptors(swapchain->swapchainImageFormat);
    auto swapchainRenderPassDescriptor = _renderPassGraphManager->getRenderPassDescriptor("swapchain");
    _uiEngine.get().setup(swapchainRenderPassDescriptor->getVkRenderPass());

    _renderPassGraphManager->setupRenderDestinations(
        *_vkEngine.get().commandManager, *_vkEngine.get().queueManager, *swapchain
    );
}

void Renderer::setup(Scene* scene) {
    for (uint32_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        if (scene != nullptr) {
            scene->prepareForDraw(engine::RenderCallContext{.currentFrame = i});
        }
        auto globalDescriptorSetWriter =
            _vkEngine.get().descriptorSetManager->getDescriptorSetWriter(CommonDescriptorSetName::GLOBAL, i);
        if (!globalDescriptorSetWriter.has_value()) {
            _logger.error("Failed to get global descriptor set writer!");
            return;
        }
        scene->writeGlobalDescriptorSet(*globalDescriptorSetWriter);
        auto shadowSunRenderDestination = _renderPassGraphManager->getRenderDestination("shadow_sun");
        globalDescriptorSetWriter->writeTexture(
            shadowSunRenderDestination->depthTexture(), GlobalDescriptorSetResourceIndex::GLOBAL_SHADOW_TEXTURE_INDEX
        );
        globalDescriptorSetWriter->update();
        auto frameDescriptorSetWriter =
            _vkEngine.get().descriptorSetManager->getDescriptorSetWriter(CommonDescriptorSetName::PER_FRAME, i);
        if (!frameDescriptorSetWriter.has_value()) {
            _logger.error("Failed to get frame descriptor set writer!");
            return;
        }
        auto agentCameraDescriptorSetWriter =
            _vkEngine.get().descriptorSetManager->getDescriptorSetWriter(CommonDescriptorSetName::AGENT_CAMERA, i);
        if (!agentCameraDescriptorSetWriter.has_value()) {
            _logger.error("Failed to get agent camera descriptor set writer!");
            return;
        }
        scene->writeFrameDescriptorSet(*frameDescriptorSetWriter, *agentCameraDescriptorSetWriter);
        agentCameraDescriptorSetWriter->update();
        frameDescriptorSetWriter->update();

        auto perDrawDescriptorSetWriter =
            _vkEngine.get().descriptorSetManager->getDescriptorSetWriter(CommonDescriptorSetName::PER_DRAW, i);
        if (!perDrawDescriptorSetWriter.has_value()) {
            _logger.error("Failed to get per draw descriptor set writer!");
            return;
        }
        scene->writePerDrawDescriptorSet(*perDrawDescriptorSetWriter);
        perDrawDescriptorSetWriter->update();
    }
    _writeFinalBlendDescriptorSets();
}

void Renderer::_writeSecondaryWindowDescriptorSets() {
    auto agentCameraRenderDestination = _renderPassGraphManager->getRenderDestination("agent_camera_main");
    for (uint32_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto agentCameraFBDescriptorSetWriter =
            _vkEngine.get().descriptorSetManager->getDescriptorSetWriter(CommonDescriptorSetName::AGENT_CAMERA_FB, i);
        if (!agentCameraFBDescriptorSetWriter.has_value()) {
            _logger.error("Failed to get agent camera FB descriptor set writer!");
            return;
        }
        agentCameraFBDescriptorSetWriter->writeTexture(agentCameraRenderDestination->textures()[i], 0, 0, true);
        agentCameraFBDescriptorSetWriter->update();
    }
}

void Renderer::_writeFinalBlendDescriptorSets() {
    auto mainRenderDestination = _renderPassGraphManager->getRenderDestination("main");
    for (uint32_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto finalBlendDescriptorSetWriter =
            _vkEngine.get().descriptorSetManager->getDescriptorSetWriter(CommonDescriptorSetName::MAIN_FB, i);
        if (!finalBlendDescriptorSetWriter.has_value()) {
            _logger.error("Failed to get post process descriptor set writer!");
            return;
        }
        finalBlendDescriptorSetWriter->writeTexture(mainRenderDestination->textures()[i], 0, 0, true);
        finalBlendDescriptorSetWriter->writeTexture(mainRenderDestination->maskTextures()[i], 1, 0, false);
        finalBlendDescriptorSetWriter->writeTexture(mainRenderDestination->depthTexture(), 2, 0, false);
        finalBlendDescriptorSetWriter->update();
    }
}

void Renderer::prepareForRender(Scene* scene) {
    ZoneScopedN("Renderer::prepareForRender");
    bool shouldRebuildGraph = renderPassGraphMode.is_dirty() || _wasResized;
    if (scene != nullptr) {
        scene->prepareForDraw(engine::RenderCallContext{.currentFrame = _currentFrame});

        if (_wasResized) {
            setup(scene);
        }

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
                    _writeSecondaryWindowDescriptorSets();
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

bool Renderer::render(Scene* scene, const FrameTime& frameTime) {
    ZoneScopedN("Renderer::render");
    _waitForFence();

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(
        _vkEngine.get().device->vkDevice,
        _vkEngine.get().getWindowController().getWindow(0)->getSwapchain()->swapchain,
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
            _vkEngine.get().device->vkDevice,
            _vkEngine.get().getWindowController().getWindow(1)->getSwapchain()->swapchain,
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
        _vkEngine.get().device->vkDevice, 1, &_vkEngine.get().syncObjectsManager->getInFlightFences()[_currentFrame]
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

    if (!VK_CHECK(vkQueueSubmit(
            _vkEngine.get().queueManager->graphicsQueue,
            1,
            &submitInfo,
            _vkEngine.get().syncObjectsManager->getInFlightFences()[_currentFrame]
        ))) {
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

    result = vkQueuePresentKHR(_vkEngine.get().queueManager->presentQueue, &presentInfo);
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

    _currentFrame = (_currentFrame + 1) % vax::vk::MAX_FRAMES_IN_FLIGHT;

    if (_frameProfiler) {
        _frameProfiler->endFrameZone("frame_workload");
    }
    return true;
}

bool Renderer::_updateCommandBuffer(
    CommandBuffer& commandBuffer, uint32_t imageIndex, uint32_t agentCameraImageIndex, vax::engine::Scene* scene
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

void Renderer::_drawUi(CommandBuffer& commandBuffer) {
    RenderPassNode::RunPassInfo runPassInfo{
        .commandBuffer = commandBuffer,
        .scene = nullptr,
        .imageIndices = _outputImageIndices,
        .frameIndex = _currentFrame,
    };

    _renderPassGraphManager->getRenderPassGraph().run(runPassInfo);
}

bool Renderer::_drawScene(CommandBuffer& commandBuffer, Scene* scene) {
    ZoneScopedN("Renderer::_drawScene");
    auto pipelineLayout = _vkEngine.get().pipelineManager->getPipelineLayout(vax::vk::PipelineLayoutName::BASE);
    if (!pipelineLayout) {
        _logger.error("Failed to get base pipeline layout!");
        return false;
    }
    if (!_bindGlobalDescriptorSet(commandBuffer, pipelineLayout))
        return false;

    RenderPassNode::RunPassInfo runPassInfo{
        .commandBuffer = commandBuffer,
        .scene = scene,
        .imageIndices = _outputImageIndices,
        .frameIndex = _currentFrame,
    };

    scene->beginDrawing(commandBuffer, _currentFrame);
    _renderPassGraphManager->getRenderPassGraph().run(runPassInfo);
    scene->endDrawing(commandBuffer, _currentFrame);
    return true;
}

bool Renderer::_bindGlobalDescriptorSet(CommandBuffer& commandBuffer, VkPipelineLayout pipelineLayout) {
    auto globalDescriptorSetHandler =
        _vkEngine.get().descriptorSetManager->getDescriptorSetHandler(CommonDescriptorSetName::GLOBAL, _currentFrame);
    if (!globalDescriptorSetHandler.has_value()) {
        _logger.error("Failed to get global descriptor set writer!");
        return false;
    }
    globalDescriptorSetHandler->bind(commandBuffer.vkCommandBuffer, pipelineLayout, MainSetIndices::GLOBAL_SET_INDEX);
    return true;
}

void Renderer::_resize() {
    auto swapchain = _getSwapchain(0);
    _renderPassGraphManager->setupRenderDestinations(
        *_vkEngine.get().commandManager, *_vkEngine.get().queueManager, *swapchain
    );
    _wasResized = true;
}

void Renderer::_resizeSecondary() {
    vkDeviceWaitIdle(_vkEngine.get().device->vkDevice);
    _getSwapchain(1)->recreate();
    _wasResized = true;
}