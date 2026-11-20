#include "renderPassGraphManager.h"
#include "jfaPass.h"
#include "renderDestinationBuilder.h"
#include "renderPassDescriptorBuilder.h"

using namespace vax::gfx;
using namespace vax::rhi;
using namespace vax;

void RenderPassGraphManager::setupRenderPassDescriptors(VkFormat imageFormat) {
    auto renderPassDescriptorBuilder = RenderPassDescriptorBuilder(_device.get());
    auto mainRenderPassDescriptor = renderPassDescriptorBuilder.buildOffscreen(imageFormat, false);
    auto shadowSunRenderPassDescriptor = renderPassDescriptorBuilder.buildShadowSun(imageFormat, false);
    auto swapchainRenderPassDescriptor = renderPassDescriptorBuilder.buildSwapchain(imageFormat);
    if (!mainRenderPassDescriptor.has_value() || !shadowSunRenderPassDescriptor.has_value() ||
        !swapchainRenderPassDescriptor.has_value()) {
        _logger.error("Failed to create main render pass descriptor!");
        return;
    }

    auto mainRenderPassDescriptorShared =
        std::make_shared<vax::rhi::RenderPassDescriptor>(std::move(mainRenderPassDescriptor.value()));
    auto shadowSunRenderPassDescriptorShared =
        std::make_shared<vax::rhi::RenderPassDescriptor>(std::move(shadowSunRenderPassDescriptor.value()));
    auto swapchainRenderPassDescriptorShared =
        std::make_shared<vax::rhi::RenderPassDescriptor>(std::move(swapchainRenderPassDescriptor.value()));

    _renderPassDescriptors["main"] = mainRenderPassDescriptorShared;
    _renderPassDescriptors["shadow_sun"] = shadowSunRenderPassDescriptorShared;
    _renderPassDescriptors["swapchain"] = swapchainRenderPassDescriptorShared;
}

void RenderPassGraphManager::setupRenderDestinations(
    vax::rhi::CommandManager& commandManager, vax::rhi::QueueManager& queueManager, vax::rhi::Swapchain& swapchain
) {
    if (_renderDestinations.contains("main") && _renderDestinations.contains("swapchain") &&
        _renderDestinations.contains("shadow_sun")) {
        // auto& mainRenderDestination = _renderDestinations.at("main");
        // auto& swapchainRenderDestination = _renderDestinations.at("swapchain");
        // auto& shadowSunRenderDestination = _renderDestinations.at("shadow_sun");
        // for (auto& colorTexture : mainRenderDestination->colorTextures()) {
        //     _pipelineManager.get().descriptorRegistry().releaseSampledTexture(colorTexture->sampledShaderHandle);
        // }
        // for (auto& maskTexture : mainRenderDestination->maskTextures()) {
        //     _pipelineManager.get().descriptorRegistry().releaseSampledTexture(maskTexture->sampledShaderHandle);
        // }
        // _pipelineManager.get().descriptorRegistry().releaseSampledTexture(
        //     mainRenderDestination->depthTexture()->sampledShaderHandle
        // );
        // for (auto& colorTexture : swapchainRenderDestination->colorTextures()) {
        //     _pipelineManager.get().descriptorRegistry().releaseSampledTexture(colorTexture->sampledShaderHandle);
        // }
        // for (auto& maskTexture : swapchainRenderDestination->maskTextures()) {
        //     _pipelineManager.get().descriptorRegistry().releaseSampledTexture(maskTexture->sampledShaderHandle);
        // }
        // _pipelineManager.get().descriptorRegistry().releaseSampledTexture(
        //     swapchainRenderDestination->depthTexture()->sampledShaderHandle
        // );
        // for (auto& colorTexture : shadowSunRenderDestination->colorTextures()) {
        //     _pipelineManager.get().descriptorRegistry().releaseSampledTexture(colorTexture->sampledShaderHandle);
        // }
        // for (auto& maskTexture : shadowSunRenderDestination->maskTextures()) {
        //     _pipelineManager.get().descriptorRegistry().releaseSampledTexture(maskTexture->sampledShaderHandle);
        // }
        // _pipelineManager.get().descriptorRegistry().releaseSampledTexture(
        //     shadowSunRenderDestination->depthTexture()->sampledShaderHandle
        // );
    }

    auto& mainRenderPassDescriptor = _renderPassDescriptors.at("main");
    auto& swapchainRenderPassDescriptor = _renderPassDescriptors.at("swapchain");
    auto& shadowSunRenderPassDescriptor = _renderPassDescriptors.at("shadow_sun");

    auto renderDestinationBuilder = RenderDestinationBuilder(_device.get(), _pipelineManager.get().descriptorRegistry());
    auto mainRenderDestination = renderDestinationBuilder.buildMainOffscreen(
        commandManager, queueManager.graphicsQueue, swapchain.swapchainExtent, *mainRenderPassDescriptor
    );
    if (!mainRenderDestination.has_value()) {
        _logger.error("Failed to create main render destination!");
        return;
    }

    auto swapchainRenderDestination = renderDestinationBuilder.buildSwapchain(
        commandManager, queueManager.graphicsQueue, swapchain, *swapchainRenderPassDescriptor
    );
    if (!swapchainRenderDestination.has_value()) {
        _logger.error("Failed to create swapchain render destination!");
        return;
    }

    auto shadowSunRenderDestination = renderDestinationBuilder.buildShadowSunOffscreen(
        commandManager, queueManager.graphicsQueue, swapchain.swapchainExtent, *shadowSunRenderPassDescriptor
    );
    if (!shadowSunRenderDestination.has_value()) {
        _logger.error("Failed to create shadow sun render destination!");
        return;
    }

    auto mainRenderDestinationShared =
        std::make_shared<vax::rhi::RenderDestination>(std::move(mainRenderDestination.value()));
    auto swapchainRenderDestinationShared =
        std::make_shared<vax::rhi::RenderDestination>(std::move(swapchainRenderDestination.value()));
    auto shadowSunRenderDestinationShared =
        std::make_shared<vax::rhi::RenderDestination>(std::move(shadowSunRenderDestination.value()));

    _renderDestinations["main"] = mainRenderDestinationShared;
    _renderDestinations["swapchain"] = swapchainRenderDestinationShared;
    _renderDestinations["shadow_sun"] = shadowSunRenderDestinationShared;
}

void RenderPassGraphManager::setupRenderDestinationsForAgentCamera(
    vax::rhi::CommandManager& commandManager, vax::rhi::QueueManager& queueManager, vax::rhi::Swapchain& swapchain
) {
    auto& mainRenderPassDescriptor = _renderPassDescriptors.at("main");
    auto& swapchainRenderPassDescriptor = _renderPassDescriptors.at("swapchain");

    auto renderDestinationBuilder = RenderDestinationBuilder(_device.get(), _pipelineManager.get().descriptorRegistry());

    auto agentCameraRenderDestination = renderDestinationBuilder.buildMainOffscreen(
        commandManager, queueManager.graphicsQueue, swapchain.swapchainExtent, *mainRenderPassDescriptor
    );
    if (!agentCameraRenderDestination.has_value()) {
        _logger.error("Failed to create main render destination!");
        return;
    }

    auto agentCameraFBRenderDestination = renderDestinationBuilder.buildSwapchain(
        commandManager, queueManager.graphicsQueue, swapchain, *swapchainRenderPassDescriptor
    );
    if (!agentCameraFBRenderDestination.has_value()) {
        _logger.error("Failed to create agent camera swapchain render destination!");
        return;
    }

    auto agentCameraRenderDestinationShared =
        std::make_shared<vax::rhi::RenderDestination>(std::move(agentCameraRenderDestination.value()));
    auto agentCameraFBRenderDestinationShared =
        std::make_shared<vax::rhi::RenderDestination>(std::move(agentCameraFBRenderDestination.value()));

    _renderDestinations["agent_camera_main"] = agentCameraRenderDestinationShared;
    _renderDestinations["agent_camera_swapchain"] = agentCameraFBRenderDestinationShared;
}

void RenderPassGraphManager::buildGraph(
    const RenderPassGraphMode& graphMode, const RenderPassGraphDescriptor* graphDescriptor
) {
    if (graphMode == RenderPassGraphMode::SCENE_GRAPH) {
        _renderPassGraph =
            _renderPassGraphBuilder->buildSceneGraph(*graphDescriptor, _renderPassDescriptors, _renderDestinations);
    } else {
        _renderPassGraph = _renderPassGraphBuilder->buildUIOnlyGraph(_renderPassDescriptors, _renderDestinations);
    }
}