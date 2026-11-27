#include "renderPassGraphBuilder.h"
#include "finalblend.h"
#include "jfaPass.h"
#include "renderPass.h"
#include "simple.h"

using namespace vax::gfx;
using namespace vax::rhi;
using namespace vax;

std::unique_ptr<RenderPassGraph> RenderPassGraphBuilder::buildUIOnlyGraph(const gfx::RenderTarget& renderTarget) {
    auto swapchain = _windowController.get().getWindow(0)->vkWindow()->swapchain();

    auto uiPass = std::make_unique<RenderPass>(
        "ui_pass",
        _device.get(),
        "ui_pass",
        renderTarget.getRenderDestination("swapchain"),
        renderTarget.getRenderPassDescriptor("swapchain")
    );
    uiPass->setRenderToSwapchain(true);
    uiPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    auto uiStep = _renderSteps["ui"];
    uiPass->addStep(uiStep);
    return std::make_unique<RenderPassGraph>(std::move(uiPass));
}

std::unique_ptr<RenderPassGraph> RenderPassGraphBuilder::buildEditorSceneGraph(const gfx::RenderTarget& renderTarget) {
    auto swapchain = _windowController.get().getWindow(0)->vkWindow()->swapchain();

    auto mainPass = std::make_unique<RenderPass>(
        "main_pass",
        _device.get(),
        "main_pass",
        renderTarget.getRenderDestination("main"),
        renderTarget.getRenderPassDescriptor("main")
    );
    mainPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});

    auto finalblendPass = std::make_unique<RenderPass>(
        "finalblend_pass",
        _device.get(),
        "finalblend_pass",
        renderTarget.getRenderDestination("swapchain"),
        renderTarget.getRenderPassDescriptor("swapchain")
    );
    finalblendPass->setRenderToSwapchain(true);
    finalblendPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    auto finalblendStep = _renderSteps["finalblend"];
    finalblendPass->addStep(finalblendStep);

    return std::make_unique<RenderPassGraph>(std::move(finalblendPass));
}

std::unique_ptr<RenderPassGraph> RenderPassGraphBuilder::buildUIOnlyGraph(
    const std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderPassDescriptor>>& renderPassDescriptors,
    const std::unordered_map<std::string, std::shared_ptr<vax::rhi::RenderDestination>>& renderDestinations
) {
    auto swapchain = _windowController.get().getWindow(0)->vkWindow()->swapchain();

    auto uiPass = std::make_unique<RenderPass>(
        "ui_pass",
        _device.get(),
        "UiPass",
        renderDestinations.at("swapchain").get(),
        renderPassDescriptors.at("swapchain").get()
    );
    uiPass->setRenderToSwapchain(true);
    uiPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    auto uiStep = std::make_unique<RenderStep>(_device.get(), _pipelineManager.get());
    uiStep->setDrawWork([&uiEngine = _uiEngine.get()](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        uiEngine.render(runPassInfo.commandBuffer.vkCommandBuffer);
    });
    uiPass->addStep(std::move(uiStep));
    return std::make_unique<RenderPassGraph>(std::move(uiPass));
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
    auto swapchain = _windowController.get().getWindow(0)->vkWindow()->swapchain();

    auto shadowPass = std::make_unique<RenderPass>(
        "shadow_pass",
        _device.get(),
        "ShadowPass",
        renderDestinations.at("shadow_sun").get(),
        renderPassDescriptors.at("shadow_sun").get()
    );
    shadowPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    auto shadowStep = std::make_unique<RenderStep>(_device.get(), _pipelineManager.get());
    shadowStep->setPipelineName("shadow");
    shadowStep->setDrawWork([](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        runPassInfo.scene->drawShadow(drawContext);
    });
    shadowPass->addStep(std::move(shadowStep));

    auto mainPass = std::make_unique<RenderPass>(
        "main_pass",
        _device.get(),
        "MainPass",
        renderDestinations.at("main").get(),
        renderPassDescriptors.at("main").get()
    );
    mainPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    auto mainStep = std::make_unique<RenderStep>(_device.get(), _pipelineManager.get());
    mainStep->setPipelineName("pbr");
    mainStep->setDrawWork([](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        runPassInfo.scene->draw(drawContext, FrameUniformSlot::MAIN_CAMERA);
    });
    mainPass->addStep(std::move(mainStep));

    auto gizmoStep = std::make_unique<RenderStep>(_device.get(), _pipelineManager.get());
    gizmoStep->setPipelineName("simple");

    gizmoStep->setDrawWork([swapchainExtent = swapchain->swapchainExtent](
                               RenderPassNode::RunPassInfo& runPassInfo, DrawContext& drawContext
                           ) {
        VkClearAttachment clearAttachment{
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .clearValue = {.depthStencil = {0.0f, 0}},
        };
        auto xOffset = static_cast<float>(swapchainExtent.width - 256);
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
    });

    if (graphDescriptor.withGizmo) {
        mainPass->addStep(std::move(gizmoStep));
    }

    auto jfaPass = std::make_unique<JFAPass>("jfa_pass", _device.get(), _pipelineManager.get());
    jfaPass->setup(renderDestinations.at("main").get());

    auto finalBlendPass = std::make_unique<RenderPass>(
        "final_blend_pass",
        _device.get(),
        "FinalBlendPass",
        renderDestinations.at("swapchain").get(),
        renderPassDescriptors.at("swapchain").get()
    );
    finalBlendPass->setRenderToSwapchain(true);
    finalBlendPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    auto finalBlendStep = std::make_unique<RenderStep>(_device.get(), _pipelineManager.get());
    finalBlendStep->setPipelineName("finalblend");
    finalBlendStep->setDrawWork([&uiEngine = _uiEngine.get(),
                                 &pipelineManager = _pipelineManager.get(),
                                 &resourceManager = _resourceManager.get(),
                                 jfa = jfaPass.get()](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        const auto& pbrSampler =
            resourceManager.samplerManager().getGlobalSampler(SamplerManager::GlobalSampler::PBRSampler);
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
            .instances = resourceManager.ssboManager().instanceBuffer(runPassInfo.frameIndex).first,
        };
        pushConstants(runPassInfo.commandBuffer, pipelineManager, finalBlendPushConstants);
        runPassInfo.scene->drawBackground(drawContext);
        uiEngine.render(runPassInfo.commandBuffer.vkCommandBuffer);
    });
    finalBlendPass->addStep(std::move(finalBlendStep));

    if (graphDescriptor.withSecondaryWindow) {
        auto agentCameraSwapchain = _windowController.get().getWindow(1)->vkWindow()->swapchain();
        auto agentCameraMainPass = std::make_unique<RenderPass>(
            "agent_camera_main_pass",
            _device.get(),
            "AgentCameraMainPass",
            renderDestinations.at("agent_camera_main").get(),
            renderPassDescriptors.at("main").get()
        );
        agentCameraMainPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = agentCameraSwapchain->swapchainExtent});
        auto agentCameraMainStep = std::make_unique<RenderStep>(_device.get(), _pipelineManager.get());
        agentCameraMainStep->setPipelineName("pbr");
        agentCameraMainStep->setDrawWork([](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
            runPassInfo.scene->draw(drawContext, FrameUniformSlot::AGENT_CAMERA);
        });
        agentCameraMainPass->addStep(std::move(agentCameraMainStep));
        auto agentCameraFBPass = std::make_unique<RenderPass>(
            "agent_camera_fb_pass",
            _device.get(),
            "AgentCameraFBPass",
            renderDestinations.at("agent_camera_swapchain").get(),
            renderPassDescriptors.at("swapchain").get()
        );
        agentCameraFBPass->setOutputImageIndex(1);
        agentCameraFBPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = agentCameraSwapchain->swapchainExtent});
        agentCameraFBPass->setRenderToSwapchain(true);
        auto agentCameraFBStep = std::make_unique<RenderStep>(_device.get(), _pipelineManager.get());
        agentCameraFBStep->setPipelineName("finalblend_cam");
        agentCameraFBStep->setDrawWork([&pipelineManager = _pipelineManager.get(),
                                        &resourceManager = _resourceManager.get()](
                                           RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext
                                       ) {
            const auto& pbrSampler =
                resourceManager.samplerManager().getGlobalSampler(SamplerManager::GlobalSampler::PBRSampler);
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
        agentCameraFBPass->addStep(std::move(agentCameraFBStep));

        jfaPass->next = std::move(finalBlendPass);

        agentCameraFBPass->next = std::move(jfaPass);

        agentCameraMainPass->next = std::move(agentCameraFBPass);

        mainPass->next = std::move(agentCameraMainPass);

        shadowPass->next = std::move(mainPass);
    } else {
        jfaPass->next = std::move(finalBlendPass);

        mainPass->next = std::move(jfaPass);

        shadowPass->next = std::move(mainPass);
    }

    return std::make_unique<RenderPassGraph>(std::move(shadowPass));
}

void RenderPassGraphBuilder::prebuild(const gfx::RenderTarget& renderTarget) {
    auto swapchain = _windowController.get().getWindow(0)->vkWindow()->swapchain();

    auto uiStep = std::make_unique<RenderStep>(_device.get(), _pipelineManager.get());
    uiStep->setDrawWork([&uiEngine = _uiEngine.get()](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        uiEngine.render(runPassInfo.commandBuffer.vkCommandBuffer);
    });

    _renderSteps["ui"] = std::move(uiStep);

    auto finalBlendStep = std::make_unique<RenderStep>(_device.get(), _pipelineManager.get());
    finalBlendStep->setPipelineName("finalblend");
    finalBlendStep->setDrawWork([&pipelineManager = _pipelineManager.get(),
                                 &resourceManager =
                                     _resourceManager.get()](RenderPass::RunPassInfo& info, DrawContext& ctx) {
        const auto& sampler =
            resourceManager.samplerManager().getGlobalSampler(SamplerManager::GlobalSampler::PBRSampler);
        if (!sampler) {
            return;
        }
        FinalBlendPushConstants push{
            .inputTextureSampler = sampler->shaderHandle,
            .inputTexture = info.frameTextures.mainColor,
            .inputMask = info.frameTextures.mainMask,
            .inputDepth = info.frameTextures.mainDepth,
            .inputJFATexture = {},
            .instances = info.sceneDrawData ? info.sceneDrawData->handles().instances : StructuredBufferShaderHandle{},
        };
        pushConstants(info.commandBuffer, pipelineManager, push);
        vkCmdDraw(info.commandBuffer.vkCommandBuffer, 3, 1, 0, 0);
    });

    _renderSteps["finalblend"] = std::move(finalBlendStep);

    auto gizmoStep = std::make_unique<RenderStep>(_device.get(), _pipelineManager.get());
    gizmoStep->setPipelineName("simple");
    gizmoStep->setDrawWork([extent = swapchain->swapchainExtent](RenderPass::RunPassInfo& info, DrawContext& ctx) {
        constexpr uint32_t gizmoSize = 256;
        const auto x = static_cast<int32_t>(extent.width - gizmoSize);
        auto cmd = info.commandBuffer.vkCommandBuffer;

        VkViewport viewport{
            .x = static_cast<float>(x),
            .y = static_cast<float>(gizmoSize),
            .width = static_cast<float>(gizmoSize),
            .height = -static_cast<float>(gizmoSize),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        VkRect2D scissor{.offset = {x, 0}, .extent = {gizmoSize, gizmoSize}};
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        VkClearAttachment clearDepth{
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .clearValue = {.depthStencil = {0.0f, 0}},
        };
        VkClearRect clearRect{.rect = scissor, .baseArrayLayer = 0, .layerCount = 1};
        vkCmdClearAttachments(cmd, 1, &clearDepth, 1, &clearRect);

        info.sceneDrawData->drawRange(
            ctx,
            "gizmo",
            FrameUniformSlot::GIZMO,
            [](const SceneDrawData::DrawRange& range, const SceneDrawData::Handles& handles) {
                return SimplePushConstants{
                    .vertices = handles.vertices,
                    .instances = handles.instances,
                    .drawData = handles.drawData,
                    .materials = handles.materials,
                    .drawIndexOffset = range.start,
                };
            }
        );
    });

    _renderSteps["gizmo"] = std::move(gizmoStep);
}