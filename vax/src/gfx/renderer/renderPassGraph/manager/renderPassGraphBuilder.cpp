#include "renderPassGraphBuilder.h"
#include "finalblend.h"
#include "jfaPass.h"
#include "renderPass.h"

using namespace vax::gfx;
using namespace vax::rhi;
using namespace vax;

std::unique_ptr<RenderPassGraph> RenderPassGraphBuilder::buildUIOnlyGraph(
    const std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderPassDescriptor>>& renderPassDescriptors,
    const std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderDestination>>& renderDestinations
) {
    auto swapchain = _windowController.get().getWindow(0)->getSwapchain();

    auto uiPass = std::make_shared<RenderPass>(
        "ui_pass",
        _device.get(),
        _pipelineManager.get(),
        "UiPass",
        renderDestinations.at("swapchain"),
        renderPassDescriptors.at("swapchain")
    );
    uiPass->setRenderToSwapchain(true);
    uiPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    uiPass->setOutsideRenderPass(true);
    uiPass->setOutsideDrawWork([&uiEngine = _uiEngine.get()](RenderPass::RunPassInfo& runPassInfo) {
        uiEngine.render(runPassInfo.commandBuffer.vkCommandBuffer);
    });
    return std::make_unique<RenderPassGraph>(uiPass, _uiEngine.get());
}

namespace {
template <typename PushConstants>
void pushConstants(
    vax::rhi::CommandBuffer& commandBuffer, const vax::rhi::PipelineManager& pipelineManager, const PushConstants& data
) {
    vkCmdPushConstants(
        commandBuffer.vkCommandBuffer, pipelineManager.pipelineLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(data), &data
    );
}
} // namespace

std::unique_ptr<RenderPassGraph> RenderPassGraphBuilder::buildSceneGraph(
    const RenderPassGraphDescriptor& graphDescriptor,
    const std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderPassDescriptor>>& renderPassDescriptors,
    const std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderDestination>>& renderDestinations
) {
    auto swapchain = _windowController.get().getWindow(0)->getSwapchain();

    auto shadowPass = std::make_shared<RenderPass>(
        "shadow_pass",
        _device.get(),
        _pipelineManager.get(),
        "ShadowPass",
        renderDestinations.at("shadow_sun"),
        renderPassDescriptors.at("shadow_sun")
    );
    shadowPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    shadowPass->setPipeline("shadow");
    shadowPass->setDrawWork([](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        runPassInfo.scene->drawShadow(drawContext);
    });

    auto mainPass = std::make_shared<RenderPass>(
        "main_pass",
        _device.get(),
        _pipelineManager.get(),
        "MainPass",
        renderDestinations.at("main"),
        renderPassDescriptors.at("main")
    );
    mainPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    mainPass->setPipeline("pbr");
    mainPass->setDrawWork([](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        runPassInfo.scene->draw(drawContext, FrameUniformSlot::MAIN_CAMERA);
    });

    auto gizmoSubpass = std::make_unique<RenderSubpass>("gizmo_subpass", _device.get(), _pipelineManager.get());

    gizmoSubpass->setDrawWork(
        [](RenderSubpass* subpass, RenderPassNode::RunPassInfo& runPassInfo, DrawContext& drawContext) {
            VkClearAttachment clearAttachment{
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .clearValue = {.depthStencil = {0.0f, 0}},
            };
            auto xOffset = static_cast<float>(subpass->getSwapchainExtent().width - 256);
            VkClearRect clearRect{
                .rect = {.offset = {static_cast<int32_t>(xOffset), 0}, .extent = {256, 256}},
                .baseArrayLayer = 0,
                .layerCount = 1,
            };

            VkViewport viewport{
                .x = xOffset,
                .y = 256.0f,
                .width = 256.0f,
                .height = -256.0f,
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };

            VkRect2D scissor{
                .offset = {static_cast<int32_t>(xOffset), 0},
                .extent = {256, 256},
            };

            vkCmdSetViewport(runPassInfo.commandBuffer.vkCommandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(runPassInfo.commandBuffer.vkCommandBuffer, 0, 1, &scissor);
            vkCmdClearAttachments(runPassInfo.commandBuffer.vkCommandBuffer, 1, &clearAttachment, 1, &clearRect);
            runPassInfo.scene->drawGizmo(drawContext);
        }
    );
    gizmoSubpass->setPipeline("simple");
    gizmoSubpass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = {256, 256}});
    gizmoSubpass->setSwapchainExtent(swapchain->swapchainExtent);

    if (graphDescriptor.withGizmo) {
        mainPass->addSubpass(std::move(gizmoSubpass));
    }

    auto jfaPass = std::make_shared<JFAPass>(
        "jfa_pass", _device.get(), _pipelineManager.get(), _resourceManager.get().textureManager()
    );
    jfaPass->setup(renderDestinations.at("main"));

    auto finalBlendPass = std::make_shared<RenderPass>(
        "final_blend_pass",
        _device.get(),
        _pipelineManager.get(),
        "FinalBlendPass",
        renderDestinations.at("swapchain"),
        renderPassDescriptors.at("swapchain")
    );
    finalBlendPass->setRenderToSwapchain(true);
    finalBlendPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    finalBlendPass->setPipeline("finalblend");
    finalBlendPass->setDrawWork([&uiEngine = _uiEngine.get(),
                                 &pipelineManager = _pipelineManager.get(),
                                 &resourceManager = _resourceManager.get(),
                                 jfa = jfaPass.get()](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        const auto& pbrSampler =
            resourceManager.textureManager().getGlobalSampler(TextureManager::GlobalSampler::PBRSampler);
        if (!pbrSampler) {
            return;
        }
        const auto& frameTextures = runPassInfo.frameTextures;
        FinalBlendPushConstants finalBlendPushConstants{
            .inputTextureSampler = pbrSampler->shaderHandle,
            .inputTexture = frameTextures.mainColor,
            .inputMask = frameTextures.mainMask,
            .inputDepth = frameTextures.mainDepth,
            .inputJFATexture = jfa->outputImage(runPassInfo),
            .instances = runPassInfo.scene->instanceBufferHandle(runPassInfo.frameIndex),
        };
        pushConstants(runPassInfo.commandBuffer, pipelineManager, finalBlendPushConstants);
        runPassInfo.scene->drawBackground(drawContext);
        uiEngine.render(runPassInfo.commandBuffer.vkCommandBuffer);
    });

    if (graphDescriptor.withSecondaryWindow) {
        auto agentCameraSwapchain = _windowController.get().getWindow(1)->getSwapchain();
        auto agentCameraMainPass = std::make_shared<RenderPass>(
            "agent_camera_main_pass",
            _device.get(),
            _pipelineManager.get(),
            "AgentCameraMainPass",
            renderDestinations.at("agent_camera_main"),
            renderPassDescriptors.at("main")
        );
        agentCameraMainPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = agentCameraSwapchain->swapchainExtent});
        agentCameraMainPass->setPipeline("pbr");
        agentCameraMainPass->setDrawWork([](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
            runPassInfo.scene->draw(drawContext, FrameUniformSlot::AGENT_CAMERA);
        });

        auto agentCameraFBPass = std::make_shared<RenderPass>(
            "agent_camera_fb_pass",
            _device.get(),
            _pipelineManager.get(),
            "AgentCameraFBPass",
            renderDestinations.at("agent_camera_swapchain"),
            renderPassDescriptors.at("swapchain")
        );
        agentCameraFBPass->setOutputImageIndex(1);
        agentCameraFBPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = agentCameraSwapchain->swapchainExtent});
        agentCameraFBPass->setPipeline("finalblend_cam");
        agentCameraFBPass->setRenderToSwapchain(true);
        agentCameraFBPass->setDrawWork([&pipelineManager = _pipelineManager.get(),
                                        &resourceManager = _resourceManager.get()](
                                           RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext
                                       ) {
            const auto& pbrSampler =
                resourceManager.textureManager().getGlobalSampler(TextureManager::GlobalSampler::PBRSampler);
            if (!pbrSampler) {
                return;
            }
            FinalBlendCamPushConstants finalBlendCamPushConstants{
                .inputTextureSampler = pbrSampler->shaderHandle,
                .inputTexture = runPassInfo.frameTextures.agentCameraColor,
            };
            pushConstants(runPassInfo.commandBuffer, pipelineManager, finalBlendCamPushConstants);
            runPassInfo.scene->drawBackground(drawContext);
        });

        shadowPass->next = mainPass;

        mainPass->next = agentCameraMainPass;

        agentCameraMainPass->next = agentCameraFBPass;

        agentCameraFBPass->next = jfaPass;

        jfaPass->next = finalBlendPass;
    } else {
        shadowPass->next = mainPass;

        mainPass->next = jfaPass;

        jfaPass->next = finalBlendPass;
    }

    return std::make_unique<RenderPassGraph>(shadowPass, _uiEngine.get());
}
