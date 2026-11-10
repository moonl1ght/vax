#include "jfaPass.h"
#include "descriptorSetLayoutBuilder.h"
#include "jfa.h"
#include "pipelineBuilder.h"
#include "shaderModule.h"
#include "textureFactory.h"
#include <vulkan/vulkan_core.h>

using namespace vax::engine;
using namespace vax::vk;

void JFAPass::setup(std::weak_ptr<vax::vk::RenderDestination> inputRenderDestination) {
    _initPipeline = _pipelineManager.get().getPipeline("jfa_init");
    _jfaPipeline = _pipelineManager.get().getPipeline("jfa_main");

    update(inputRenderDestination);
}

void JFAPass::update(std::weak_ptr<vax::vk::RenderDestination> inputRenderDestination) {
    _inputRenderDestination = std::move(inputRenderDestination);
    if (auto inputRenderDestinationShared = _inputRenderDestination.lock()) {
        _writeTextures(inputRenderDestinationShared->maskTextures(), inputRenderDestinationShared->depthTexture());
    }
}

void JFAPass::_writeTextures(const std::vector<vax::vk::Texture>& maskTextures, const vax::vk::Texture& depthTexture) {
    _jfaTexturesA.clear();
    _jfaTexturesB.clear();
    auto textureFactory = TextureFactory(_device.get());
    for (int i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto& initDescriptorSetWriter = _initPipeline->descriptorSetWriters["JFAInit"][i];
        auto& jfaDescriptorSetWriter = _jfaPipeline->descriptorSetWriters["JFAStep"][i];

        initDescriptorSetWriter.writeTexture(maskTextures[i], 0);
        initDescriptorSetWriter.writeTexture(depthTexture, 1);

        auto inputTextureSize = maskTextures[i].size();
        auto textureA = textureFactory.makeTextureDetached(
            TextureFactory::TextureCreateInfo{
            .name = "jfa_texture_a_" + std::to_string(i),
            .format = VK_FORMAT_R16G16_UINT,
            .size = inputTextureSize,
            .imageUsageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .flags = 0,
            }
        );
        if (!textureA) {
            _logger.error("Failed to create JFA texture A!");
            return;
        }
        textureA->loadImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1);
        _jfaTexturesA.push_back(std::move(*textureA));
        initDescriptorSetWriter.writeStorageTexture(_jfaTexturesA[i], 2);

        auto textureB = textureFactory.makeTextureDetached(
            TextureFactory::TextureCreateInfo{
            .name = "jfa_texture_b_" + std::to_string(i),
            .format = VK_FORMAT_R16G16_UINT,
            .size = inputTextureSize,
            .imageUsageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .flags = 0,
            }
        );
        if (!textureB) {
            _logger.error("Failed to create JFA texture B!");
            return;
        }
        textureB->loadImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1);
        _jfaTexturesB.push_back(std::move(*textureB));

        jfaDescriptorSetWriter.writeStorageTexture(_jfaTexturesA[i], 0);
        jfaDescriptorSetWriter.writeStorageTexture(_jfaTexturesB[i], 1);

        jfaDescriptorSetWriter.update();
        initDescriptorSetWriter.update();
    }
}

void JFAPass::runPass(RunPassInfo& runPassInfo) {
    if (!_initPipeline.has_value() || !_jfaPipeline.has_value()) {
        _logger.error("JFA pipelines are not initialized!");
        return;
    }

    VkImageMemoryBarrier2 textureABarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .image = _jfaTexturesA[runPassInfo.frameIndex].image(),
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    VkDependencyInfo dependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &textureABarrier,
    };

    vkCmdPipelineBarrier2(runPassInfo.commandBuffer.vkCommandBuffer, &dependencyInfo);

    vkCmdBindPipeline(
        runPassInfo.commandBuffer.vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _initPipeline->pipeline->vkPipeline
    );

    auto& initDescriptorSetHandler = _initPipeline->descriptorSetHandlers["JFAInit"][runPassInfo.frameIndex];
    initDescriptorSetHandler.bind(
        runPassInfo.commandBuffer.vkCommandBuffer, _initPipeline->pipelineLayout, 0, VK_PIPELINE_BIND_POINT_COMPUTE
    );

    uint32_t width = _jfaTexturesA[runPassInfo.frameIndex].width();
    uint32_t height = _jfaTexturesA[runPassInfo.frameIndex].height();

    uint32_t groupCountX = (width + 15) / 16;
    uint32_t groupCountY = (height + 15) / 16;
    vkCmdDispatch(runPassInfo.commandBuffer.vkCommandBuffer, groupCountX, groupCountY, 1);

    VkImageMemoryBarrier2 prepareBarriers[2]{
        {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .image = _jfaTexturesA[runPassInfo.frameIndex].image(),
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        },
        {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .image = _jfaTexturesB[runPassInfo.frameIndex].image(),
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        }
    };

    VkDependencyInfo prepareDependency{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 2,
        .pImageMemoryBarriers = prepareBarriers,
    };
    vkCmdPipelineBarrier2(runPassInfo.commandBuffer.vkCommandBuffer, &prepareDependency);

    vkCmdBindPipeline(
        runPassInfo.commandBuffer.vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _jfaPipeline->pipeline->vkPipeline
    );

    bool isTextureAInput = true;
    uint32_t maxDim = std::max(width, height);

    auto& jfaDescriptorSetHandler = _jfaPipeline->descriptorSetHandlers["JFAStep"][runPassInfo.frameIndex];

    jfaDescriptorSetHandler.bind(
        runPassInfo.commandBuffer.vkCommandBuffer, _jfaPipeline->pipelineLayout, 0, VK_PIPELINE_BIND_POINT_COMPUTE
    );

    for (uint32_t step = maxDim / 2; step > 0; step /= 2) {
        JFAPushConstants pushConstants{.step = step, .swapInputAndOutput = isTextureAInput ? 0u : 1u};

        vkCmdPushConstants(
            runPassInfo.commandBuffer.vkCommandBuffer,
            _jfaPipeline->pipelineLayout,
            VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            sizeof(pushConstants),
            &pushConstants
        );

        vkCmdDispatch(runPassInfo.commandBuffer.vkCommandBuffer, groupCountX, groupCountY, 1);

        VkImageMemoryBarrier2 loopBarriers[2]{
            {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .image = isTextureAInput ? _jfaTexturesB[runPassInfo.frameIndex].image()
                                     : _jfaTexturesA[runPassInfo.frameIndex].image(),
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            },
            {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .image = isTextureAInput ? _jfaTexturesA[runPassInfo.frameIndex].image()
                                     : _jfaTexturesB[runPassInfo.frameIndex].image(),
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            }
        };

        VkDependencyInfo loopDependency{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 2,
            .pImageMemoryBarriers = loopBarriers,
        };
        vkCmdPipelineBarrier2(runPassInfo.commandBuffer.vkCommandBuffer, &loopDependency);

        isTextureAInput = !isTextureAInput;
    }

    VkImage finalImage =
        isTextureAInput ? _jfaTexturesA[runPassInfo.frameIndex].image() : _jfaTexturesB[runPassInfo.frameIndex].image();

    VkImageMemoryBarrier2 finalBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .image = finalImage,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    VkDependencyInfo finalDependency{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &finalBarrier,
    };
    vkCmdPipelineBarrier2(runPassInfo.commandBuffer.vkCommandBuffer, &finalDependency);
    _isFinalImageA = isTextureAInput;
}

const std::vector<vax::vk::Texture>& JFAPass::outputATextures() const { return _jfaTexturesA; }

const std::vector<vax::vk::Texture>& JFAPass::outputBTextures() const { return _jfaTexturesB; }