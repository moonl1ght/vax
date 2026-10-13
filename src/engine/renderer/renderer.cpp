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

void Renderer::setup(DrawableScene* scene) {
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
        auto shadowSunRenderDestination =
            _renderPassGraphManager->getRenderDestination("shadow_sun");
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
        auto roverCameraDescriptorSetWriter =
            _vkEngine.get().descriptorSetManager->getDescriptorSetWriter(CommonDescriptorSetName::ROVER_CAMERA, i);
        if (!roverCameraDescriptorSetWriter.has_value()) {
            _logger.error("Failed to get rover camera descriptor set writer!");
            return;
        }
        scene->writeFrameDescriptorSet(*frameDescriptorSetWriter, *roverCameraDescriptorSetWriter);
        roverCameraDescriptorSetWriter->update();
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
    auto roverCameraRenderDestination =
        _renderPassGraphManager->getRenderDestination("rover_camera_main");
    for (uint32_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto roverCameraFBDescriptorSetWriter =
            _vkEngine.get().descriptorSetManager->getDescriptorSetWriter(CommonDescriptorSetName::ROVER_CAMERA_FB, i);
        if (!roverCameraFBDescriptorSetWriter.has_value()) {
            _logger.error("Failed to get rover camera FB descriptor set writer!");
            return;
        }
        roverCameraFBDescriptorSetWriter->writeTexture(roverCameraRenderDestination->textures()[i], 0, 0, true);
        roverCameraFBDescriptorSetWriter->update();
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

void Renderer::prepareForRender(DrawableScene* scene) {
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
                    _renderPassGraphManager->setupRenderDestinationsForRoverCamera(
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

bool Renderer::render(DrawableScene* scene, const FrameTime& frameTime) {
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
    uint32_t roverCameraImageIndex = UINT32_MAX;
    if (drawSecondaryWindow) {
        VkResult roverCameraResult = vkAcquireNextImageKHR(
            _vkEngine.get().device->vkDevice,
            _vkEngine.get().getWindowController().getWindow(1)->getSwapchain()->swapchain,
            UINT64_MAX,
            _vkEngine.get().syncObjectsManager->getImageAvailableSemaphores("secondary")[_currentFrame],
            VK_NULL_HANDLE,
            &roverCameraImageIndex
        );
        if (roverCameraResult != VK_SUCCESS && roverCameraResult != VK_SUBOPTIMAL_KHR) {
            _logger.warning("Failed to acquire rover camera swap chain image, skipping secondary window this frame");
            drawSecondaryWindow = false;
            roverCameraImageIndex = UINT32_MAX;
        }
    }

    _outputImageIndices = {imageIndex, roverCameraImageIndex};

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

    auto updateResult = _updateCommandBuffer(commandBuffer, imageIndex, roverCameraImageIndex, scene);
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
        imageIndices.push_back(roverCameraImageIndex);
    }
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = static_cast<uint32_t>(swapChains.size()),
        .pSwapchains = swapChains.data(),
        .pImageIndices = imageIndices.data()
    };

    result = vkQueuePresentKHR(_vkEngine.get().queueManager->presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _vkEngine.get().framebufferResized) {
        _vkEngine.get().framebufferResized = false;
        _vkEngine.get().resize();
        _resize();
        if (scene != nullptr) {
            scene->resize();
        }
        return false;
    } else if (result != VK_SUCCESS) {
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
    CommandBuffer& commandBuffer, uint32_t imageIndex, uint32_t roverCameraImageIndex, vax::engine::DrawableScene* scene
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

bool Renderer::_drawScene(CommandBuffer& commandBuffer, DrawableScene* scene) {
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