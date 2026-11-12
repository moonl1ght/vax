#include "jfaPass.h"
#include "jfa.h"
#include "textureFactory.h"
#include <vulkan/vulkan_core.h>

using namespace vax::engine;
using namespace vax::vk;

void JFAPass::setup(std::weak_ptr<vax::vk::RenderDestination> inputRenderDestination) {
    _initPipeline = _pipelineManager.get().getPipelineV2("jfa_init");
    _stepPipeline = _pipelineManager.get().getPipelineV2("jfa_step");

    update(inputRenderDestination);
}

void JFAPass::update(std::weak_ptr<vax::vk::RenderDestination> inputRenderDestination) {
    _inputRenderDestination = std::move(inputRenderDestination);
    if (auto inputRenderDestinationShared = _inputRenderDestination.lock()) {
        auto& descriptorRegistry = _pipelineManager.get().descriptorRegistry();
        for (const auto& textureHandle : _textureHandles) {
            descriptorRegistry.releaseStorageTexture(textureHandle.jfaA);
            descriptorRegistry.releaseStorageTexture(textureHandle.jfaB);
            descriptorRegistry.releaseSampledTexture(textureHandle.mask);
            descriptorRegistry.releaseSampledTexture(textureHandle.depth);
        }
        _createTextures(inputRenderDestinationShared->maskTextures(), inputRenderDestinationShared->depthTexture());
    }
}

void JFAPass::_createTextures(const std::vector<vax::vk::Texture>& maskTextures, const vax::vk::Texture& depthTexture) {
    _jfaTexturesA.clear();
    _jfaTexturesB.clear();
    _textureHandles.clear();
    _textureHandles.resize(vax::vk::MAX_FRAMES_IN_FLIGHT);
    auto textureFactory = TextureFactory(_device.get());
    for (int i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {

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
        auto& descriptorRegistry = _pipelineManager.get().descriptorRegistry();
        auto registrationResult = descriptorRegistry.registerBatch({
            .storageTextures = std::array<const Texture*, 2>{&_jfaTexturesA[i], &_jfaTexturesB[i]},
            .sampledTextures = std::array<const Texture*, 2>{&maskTextures[i], &depthTexture},
        });
        _textureHandles[i] = TextureHandles{
            .mask = registrationResult.sampledTextures.value()[0],
            .depth = registrationResult.sampledTextures.value()[1],
            .jfaA = registrationResult.storageTextures.value()[0],
            .jfaB = registrationResult.storageTextures.value()[1],
        };
    }
}

void JFAPass::runPass(RunPassInfo& runPassInfo) {
    if (!_initPipeline.has_value() || !_stepPipeline.has_value()) {
        _logger.error("JFA pipelines are not initialized!");
        return;
    }

    auto& textureHandle = _textureHandles[runPassInfo.frameIndex];

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
        runPassInfo.commandBuffer.vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _initPipeline.value().vkPipeline
    );
    JFAInitPushConstants initPushConstants{
        .inputMask = textureHandle.mask, .inputDepth = textureHandle.depth, .output = textureHandle.jfaA
    };
    vkCmdPushConstants(
        runPassInfo.commandBuffer.vkCommandBuffer,
        layout,
        VK_SHADER_STAGE_ALL,
        0,
        sizeof(initPushConstants),
        &initPushConstants
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
        runPassInfo.commandBuffer.vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _stepPipeline.value().vkPipeline
    );

    bool isTextureAInput = true;
    uint32_t maxDim = std::max(width, height);

    StorageImageShaderHandle in = textureHandle.jfaA, out = textureHandle.jfaB;
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
        std::swap(in, out);
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