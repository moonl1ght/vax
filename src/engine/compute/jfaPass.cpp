#include "jfaPass.h"
#include "jfa.h"
#include <vulkan/vulkan_core.h>

using namespace vax::engine;
using namespace vax::vk;

void JFAPass::setup(std::weak_ptr<vax::vk::RenderDestination> inputRenderDestination) {
    _initPipeline = _pipelineManager.get().getPipeline("jfa_init");
    _stepPipeline = _pipelineManager.get().getPipeline("jfa_step");

    update(inputRenderDestination);
}

void JFAPass::update(std::weak_ptr<vax::vk::RenderDestination> inputRenderDestination) {
    _inputRenderDestination = std::move(inputRenderDestination);
    if (auto inputRenderDestinationShared = _inputRenderDestination.lock()) {
        auto& descriptorRegistry = _pipelineManager.get().descriptorRegistry();
        _inputMaskTextures = inputRenderDestinationShared->maskTextures();
        _inputDepthTextures = inputRenderDestinationShared->depthTexture();
        _createTextures(inputRenderDestinationShared->maskTextures(), inputRenderDestinationShared->depthTexture());
    }
}

void JFAPass::_createTextures(
    const std::vector<vax::vk::TextureManager::TextureResource*>& maskTextures,
    const vax::vk::TextureManager::TextureResource* depthTexture
) {
    _jfaTexturesA.clear();
    _jfaTexturesB.clear();
    for (int i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {

        auto inputTextureSize = maskTextures[i]->texture.size();
        auto textureCreateInfo = TextureManager::TextureCreateInfo{
            .registrationType = vax::vk::TextureManager::TextureRegistrationType::Both,
            .name = "jfa_texture_a_" + std::to_string(i),
            .format = VK_FORMAT_R16G16_UINT,
            .size = inputTextureSize,
            .imageUsageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .flags = 0,
        };
        auto textureA = _textureManager.get().createTexture(textureCreateInfo);
        if (!textureA) {
            _logger.error("Failed to create JFA texture A!");
            return;
        }
        textureA->texture.loadImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1);
        _jfaTexturesA.push_back(textureA);

        textureCreateInfo.name = "jfa_texture_b_" + std::to_string(i);
        auto textureB = _textureManager.get().createTexture(textureCreateInfo);
        if (!textureB) {
            _logger.error("Failed to create JFA texture B!");
            return;
        }
        textureB->texture.loadImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1);
        _jfaTexturesB.push_back(textureB);
    }
}

void JFAPass::runPass(RunPassInfo& runPassInfo) {
    if (!_initPipeline.has_value() || !_stepPipeline.has_value()) {
        _logger.error("JFA pipelines are not initialized!");
        return;
    }

    VkPipelineLayout layout = _initPipeline.value().pipelineLayout;
    VkDescriptorSet set = _initPipeline.value().descriptorSetLayout;
    vkCmdBindDescriptorSets(
        runPassInfo.commandBuffer.vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, layout, 0, 1, &set, 0, nullptr
    );

    VkImageMemoryBarrier2 textureABarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .image = _jfaTexturesA[runPassInfo.frameIndex]->texture.image(),
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    VkDependencyInfo dependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &textureABarrier,
    };

    vkCmdPipelineBarrier2(runPassInfo.commandBuffer.vkCommandBuffer, &dependencyInfo);

    vkCmdBindPipeline(
        runPassInfo.commandBuffer.vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _initPipeline.value().vkPipeline
    );
    auto inputMask = _inputMaskTextures[runPassInfo.frameIndex]->sampledShaderHandle;
    auto inputDepth = _inputDepthTextures->sampledShaderHandle;
    auto output = _jfaTexturesA[runPassInfo.frameIndex]->storageShaderHandle;
    JFAInitPushConstants initPushConstants{.inputMask = inputMask, .inputDepth = inputDepth, .output = output};
    vkCmdPushConstants(
        runPassInfo.commandBuffer.vkCommandBuffer,
        layout,
        VK_SHADER_STAGE_ALL,
        0,
        sizeof(initPushConstants),
        &initPushConstants
    );

    uint32_t width = _jfaTexturesA[runPassInfo.frameIndex]->texture.width();
    uint32_t height = _jfaTexturesA[runPassInfo.frameIndex]->texture.height();

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
        .image = _jfaTexturesA[runPassInfo.frameIndex]->texture.image(),
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
        .image = _jfaTexturesB[runPassInfo.frameIndex]->texture.image(),
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
        runPassInfo.commandBuffer.vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _stepPipeline.value().vkPipeline
    );

    bool isTextureAInput = true;
    uint32_t maxDim = std::max(width, height);

    StorageImageShaderHandle in = _jfaTexturesA[runPassInfo.frameIndex]->storageShaderHandle;
    StorageImageShaderHandle out = _jfaTexturesB[runPassInfo.frameIndex]->storageShaderHandle;
    for (uint32_t step = maxDim / 2; step > 0; step /= 2) {
        JFAStepPushConstants stepPushConstants{.input = in, .output = out, .step = step};
        vkCmdPushConstants(
            runPassInfo.commandBuffer.vkCommandBuffer,
            layout,
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(stepPushConstants),
            &stepPushConstants
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
            .image = isTextureAInput ? _jfaTexturesB[runPassInfo.frameIndex]->texture.image()
                                     : _jfaTexturesA[runPassInfo.frameIndex]->texture.image(),
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
            .image = isTextureAInput ? _jfaTexturesA[runPassInfo.frameIndex]->texture.image()
                                     : _jfaTexturesB[runPassInfo.frameIndex]->texture.image(),
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
        std::swap(in, out);
    }

    VkImage finalImage = isTextureAInput ? _jfaTexturesA[runPassInfo.frameIndex]->texture.image()
                                         : _jfaTexturesB[runPassInfo.frameIndex]->texture.image();

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