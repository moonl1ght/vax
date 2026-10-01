#include "jfaPass.h"
#include "renderPass.h"
#include "renderPassGraphManager.h"

using namespace vax::engine;
using namespace vax::vk;

std::unique_ptr<RenderPassGraph> RenderPassGraphManager::buildRoverDemoGraph(bool withRoverCamera) {
    auto swapchain = _windowController.get().getWindow(0)->getSwapchain();

    auto shadowPass = std::make_shared<RenderPass>(
        "shadow_pass",
        _device.get(),
        _pipelineManager.get(),
        _descriptorSetManager.get(),
        "ShadowPass",
        _renderDestinations.at("shadow_sun"),
        _renderPassDescriptors.at("shadow_sun")
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
        _renderDestinations.at("main"),
        _renderPassDescriptors.at("main")
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

    // mainPass->addSubpass(std::move(gizmoSubpass));

    auto jfaPass = std::make_shared<JFAPass>("jfa_pass", _device.get(), _descriptorSetManager.get());
    jfaPass->setup(_renderDestinations.at("main"));

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
        _renderDestinations.at("swapchain"),
        _renderPassDescriptors.at("swapchain")
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

    if (withRoverCamera) {
        auto roverCameraSwapchain = _windowController.get().getWindow(1)->getSwapchain();
        auto roverCameraMainPass = std::make_shared<RenderPass>(
            "rover_camera_main_pass",
            _device.get(),
            _pipelineManager.get(),
            _descriptorSetManager.get(),
            "RoverCameraMainPass",
            _renderDestinations.at("rover_camera_main"),
            _renderPassDescriptors.at("main")
        );
        roverCameraMainPass->addInputDescriptorSet({
            .setName = CommonDescriptorSetName::ROVER_CAMERA,
            .bindingInfo = {
            .setIndex = MainSetIndices::PER_FRAME_SET_INDEX,
            .bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .dynamicOffsetCount = 1,
            .dynamicOffsets = {0},
            },
        });
        roverCameraMainPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = roverCameraSwapchain->swapchainExtent});
        roverCameraMainPass->setPipeline(vax::vk::PipelineName::ROVER_CAMERA);
        roverCameraMainPass->setPipelineLayout(vax::vk::PipelineLayoutName::BASE);
        roverCameraMainPass->setDrawWork([this](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
            runPassInfo.scene->draw(drawContext);
        });

        auto roverCameraFBPass = std::make_shared<RenderPass>(
            "rover_camera_fb_pass",
            _device.get(),
            _pipelineManager.get(),
            _descriptorSetManager.get(),
            "RoverCameraFBPass",
            _renderDestinations.at("rover_camera_swapchain"),
            _renderPassDescriptors.at("swapchain")
        );
        roverCameraFBPass->addInputDescriptorSet({
            .setName = CommonDescriptorSetName::ROVER_CAMERA_FB,
            .bindingInfo = {
            .setIndex = 0,
            .bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .dynamicOffsetCount = 1,
            .dynamicOffsets = {0},
            },
        });
        roverCameraFBPass->setOutputImageIndex(1);
        roverCameraFBPass->setRenderArea(VkRect2D{.offset = {0, 0}, .extent = roverCameraSwapchain->swapchainExtent});
        roverCameraFBPass->setPipeline(vax::vk::PipelineName::ROVER_CAMERA_FB);
        roverCameraFBPass->setPipelineLayout(vax::vk::PipelineLayoutName::ROVER_CAMERA_FB);
        roverCameraFBPass->setRenderToSwapchain(true);
        roverCameraFBPass->setDrawWork([this](RenderPass::RunPassInfo& runPassInfo, DrawContext& drawContext) {
            runPassInfo.scene->drawBackground(drawContext);
        });

        shadowPass->next = mainPass;

        mainPass->next = roverCameraMainPass;

        roverCameraMainPass->next = roverCameraFBPass;

        roverCameraFBPass->next = jfaPass;

        jfaPass->next = finalBlendPass;
    } else {
        shadowPass->next = mainPass;

        mainPass->next = jfaPass;

        jfaPass->next = finalBlendPass;
    }

    auto renderPassGraph =
        std::make_unique<RenderPassGraph>(shadowPass, _renderPassDescriptors, _renderDestinations, _uiEngine.get());
    return renderPassGraph;
}