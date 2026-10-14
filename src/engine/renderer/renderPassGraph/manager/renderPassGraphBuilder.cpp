#include "renderPassGraphBuilder.h"
#include "jfaPass.h"
#include "renderPass.h"
#include "commonDescriptorSets.h"

using namespace vax::engine;
using namespace vax::vk;
using namespace vax;

std::unique_ptr<RenderPassGraph> RenderPassGraphBuilder::buildUIOnlyGraph(
    const std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderPassDescriptor>>& renderPassDescriptors,
    const std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderDestination>>& renderDestinations
) {
    auto swapchain = _windowController.get().getWindow(0)->getSwapchain();

    auto uiPass = std::make_shared<RenderPass>(
        "ui_pass",
        _device.get(),
        _pipelineManager.get(),
        _descriptorSetManager.get(),
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

std::unique_ptr<RenderPassGraph> RenderPassGraphBuilder::buildSceneGraph(
    const RenderPassGraphDescriptor& graphDescriptor,
    const std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderPassDescriptor>>& renderPassDescriptors,
    const std::unordered_map<std::string, std::shared_ptr<vax::vk::RenderDestination>>& renderDestinations
) {
    auto swapchain = _windowController.get().getWindow(0)->getSwapchain();

    auto shadowPass = std::make_shared<RenderPass>(
        "shadow_pass",
        _device.get(),
        _pipelineManager.get(),
        _descriptorSetManager.get(),
        "ShadowPass",
        renderDestinations.at("shadow_sun"),
        renderPassDescriptors.at("shadow_sun")
    );
    shadowPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    shadowPass->setPipeline(vax::vk::PipelineName::SHADOW);
    shadowPass->setPipelineLayout(vax::vk::PipelineLayoutName::BASE);
    shadowPass->setDrawWork([](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        runPassInfo.scene->draw(drawContext);
    });
    auto dynamicOffset = static_cast<uint32_t>(_device.get().minUniformBufferOffsetAlignment<UniformBufferObject>());
    shadowPass->addInputDescriptorSet({
        .setName = CommonDescriptorSetName::PER_FRAME,
        .bindingInfo = {
        .setIndex = MainSetIndices::PER_FRAME_SET_INDEX,
        .bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .dynamicOffsetCount = 1,
        .dynamicOffsets = {dynamicOffset},
        },
    });

    auto mainPass = std::make_shared<RenderPass>(
        "main_pass",
        _device.get(),
        _pipelineManager.get(),
        _descriptorSetManager.get(),
        "MainPass",
        renderDestinations.at("main"),
        renderPassDescriptors.at("main")
    );
    mainPass->addInputDescriptorSet({
        .setName = CommonDescriptorSetName::PER_FRAME,
        .bindingInfo = {
        .setIndex = MainSetIndices::PER_FRAME_SET_INDEX,
        .bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .dynamicOffsetCount = 1,
        .dynamicOffsets = {0},
        },
    });
    mainPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    mainPass->setPipeline(vax::vk::PipelineName::PBR);
    mainPass->setPipelineLayout(vax::vk::PipelineLayoutName::BASE);
    mainPass->setDrawWork([](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        runPassInfo.scene->draw(drawContext);
    });

    auto gizmoSubpass = std::make_unique<RenderSubpass>(
        "gizmo_subpass", _device.get(), _pipelineManager.get(), _descriptorSetManager.get()
    );

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
    gizmoSubpass->setPipeline(vax::vk::PipelineName::BASE);
    gizmoSubpass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = {256, 256}});
    gizmoSubpass->setSwapchainExtent(swapchain->swapchainExtent);

    if (graphDescriptor.withGizmo) {
        mainPass->addSubpass(std::move(gizmoSubpass));
    }

    auto jfaPass = std::make_shared<JFAPass>("jfa_pass", _device.get(), _descriptorSetManager.get());
    jfaPass->setup(renderDestinations.at("main"));

    jfaPass->setPostPassWork([](RenderPassNode* passNode, RenderPass::RunPassInfo& runPassInfo) {
        auto jfaPass = dynamic_cast<JFAPass*>(passNode);
        if (!jfaPass) {
            return;
        }
        if (jfaPass->next) {
            auto finalBlendPass = dynamic_cast<RenderPass*>(jfaPass->next.get());
            if (!finalBlendPass) {
                return;
            }
            auto descriptorSetInfo = finalBlendPass->getInputDescriptorSetAt(1);
            descriptorSetInfo.setName = jfaPass->outputDescriptorSetName();
            finalBlendPass->updateInputDescriptorSetAt(1, descriptorSetInfo);
        }
    });

    auto finalBlendPass = std::make_shared<RenderPass>(
        "final_blend_pass",
        _device.get(),
        _pipelineManager.get(),
        _descriptorSetManager.get(),
        "FinalBlendPass",
        renderDestinations.at("swapchain"),
        renderPassDescriptors.at("swapchain")
    );
    finalBlendPass->addInputDescriptorSet({
        .setName = CommonDescriptorSetName::MAIN_FB,
        .bindingInfo = {
        .setIndex = 0,
        .bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .dynamicOffsetCount = 1,
        .dynamicOffsets = {0},
        },
    });
    finalBlendPass->addInputDescriptorSet({
        .setName = jfaPass->outputDescriptorSetName(),
        .bindingInfo = {
        .setIndex = 1,
        .bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .dynamicOffsetCount = 1,
        .dynamicOffsets = {0},
        },
    });
    finalBlendPass->addInputDescriptorSet({
        .setName = CommonDescriptorSetName::PER_FRAME,
        .bindingInfo = {
        .setIndex = 2,
        .bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .dynamicOffsetCount = 1,
        .dynamicOffsets = {0},
        },
    });
    finalBlendPass->setRenderToSwapchain(true);
    finalBlendPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = swapchain->swapchainExtent});
    finalBlendPass->setPipeline(vax::vk::PipelineName::FINAL_BLEND);
    finalBlendPass->setPipelineLayout(vax::vk::PipelineLayoutName::FINAL_BLEND);
    finalBlendPass->setDrawWork([&uiEngine =
                                     _uiEngine.get()](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
        runPassInfo.scene->drawBackground(drawContext);
        uiEngine.render(runPassInfo.commandBuffer.vkCommandBuffer);
    });

    if (graphDescriptor.withSecondaryWindow) {
        auto agentCameraSwapchain = _windowController.get().getWindow(1)->getSwapchain();
        auto agentCameraMainPass = std::make_shared<RenderPass>(
            "agent_camera_main_pass",
            _device.get(),
            _pipelineManager.get(),
            _descriptorSetManager.get(),
            "AgentCameraMainPass",
            renderDestinations.at("agent_camera_main"),
            renderPassDescriptors.at("main")
        );
        agentCameraMainPass->addInputDescriptorSet({
            .setName = CommonDescriptorSetName::AGENT_CAMERA,
            .bindingInfo = {
            .setIndex = MainSetIndices::PER_FRAME_SET_INDEX,
            .bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .dynamicOffsetCount = 1,
            .dynamicOffsets = {0},
            },
        });
        agentCameraMainPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = agentCameraSwapchain->swapchainExtent});
        agentCameraMainPass->setPipeline(vax::vk::PipelineName::AGENT_CAMERA);
        agentCameraMainPass->setPipelineLayout(vax::vk::PipelineLayoutName::BASE);
        agentCameraMainPass->setDrawWork([this](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
            runPassInfo.scene->draw(drawContext);
        });

        auto agentCameraFBPass = std::make_shared<RenderPass>(
            "agent_camera_fb_pass",
            _device.get(),
            _pipelineManager.get(),
            _descriptorSetManager.get(),
            "AgentCameraFBPass",
            renderDestinations.at("agent_camera_swapchain"),
            renderPassDescriptors.at("swapchain")
        );
        agentCameraFBPass->addInputDescriptorSet({
            .setName = CommonDescriptorSetName::AGENT_CAMERA_FB,
            .bindingInfo = {
            .setIndex = 0,
            .bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .dynamicOffsetCount = 1,
            .dynamicOffsets = {0},
            },
        });
        agentCameraFBPass->setOutputImageIndex(1);
        agentCameraFBPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = agentCameraSwapchain->swapchainExtent});
        agentCameraFBPass->setPipeline(vax::vk::PipelineName::AGENT_CAMERA_FB);
        agentCameraFBPass->setPipelineLayout(vax::vk::PipelineLayoutName::AGENT_CAMERA_FB);
        agentCameraFBPass->setRenderToSwapchain(true);
        agentCameraFBPass->setDrawWork([this](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
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