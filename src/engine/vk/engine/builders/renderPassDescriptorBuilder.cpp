#include "renderPassDescriptorBuilder.h"
#include "device.h"
#include "luna.h"
#include "shaderUniforms.h"
#include "vkUtils.h"

using namespace vax::vk;

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
    return findSupportedFormat(
        physicalDevice, {VK_FORMAT_D32_SFLOAT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

std::optional<vax::vk::RenderPassDescriptor>
RenderPassDescriptorBuilder::buildSwapchain(VkFormat imageFormat) const noexcept {
    VkAttachmentDescription colorAttachment{
        .format = imageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkFormat depthFormat = findDepthFormat(_device.get().vkPhysicalDevice);
    VkAttachmentDescription depthAttachment{
        .format = depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depthAttachmentRef{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };

    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VkRenderPass renderPass;
    if (!VK_CHECK(vkCreateRenderPass(_device.get().vkDevice, &renderPassInfo, nullptr, &renderPass))) {
        _logger.error("Failed to create render pass!");
        return std::nullopt;
    }

    return std::make_optional<vax::vk::RenderPassDescriptor>(_device.get(), renderPass, imageFormat, depthFormat);
}

std::optional<vax::vk::RenderPassDescriptor>
RenderPassDescriptorBuilder::buildOffscreen(VkFormat imageFormat, bool allowComputeUsage) const noexcept {
    VkAttachmentDescription colorAttachment{
        .format = imageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = allowComputeUsage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkAttachmentDescription maskAttachment{
        .format = VK_FORMAT_R32_UINT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkFormat depthFormat = findDepthFormat(_device.get().vkPhysicalDevice);
    VkAttachmentDescription depthAttachment{
        .format = depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
    };

    std::array<VkAttachmentReference, 2> colorAttachmentRefs{
        VkAttachmentReference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
        VkAttachmentReference{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
    };

    VkAttachmentReference depthAttachmentRef{
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size()),
        .pColorAttachments = colorAttachmentRefs.data(),
        .pDepthStencilAttachment = &depthAttachmentRef,
    };

    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0] = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    VkPipelineStageFlags dstStageMask =
        allowComputeUsage ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                          : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1] = {
        .srcSubpass = 0,
        .dstSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        .dstStageMask = dstStageMask,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, maskAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = static_cast<uint32_t>(dependencies.size()),
        .pDependencies = dependencies.data(),
    };

    VkRenderPass renderPass;
    if (!VK_CHECK(vkCreateRenderPass(_device.get().vkDevice, &renderPassInfo, nullptr, &renderPass))) {
        _logger.error("Failed to create render pass!");
        return std::nullopt;
    }

    std::vector<VkClearValue> colorClearValues;
    for (size_t i = 0; i < colorAttachmentRefs.size(); ++i) {
        colorClearValues.push_back(VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}});
    }
    return std::make_optional<vax::vk::RenderPassDescriptor>(
        _device.get(),
        renderPass,
        imageFormat,
        depthFormat,
        static_cast<uint32_t>(colorAttachmentRefs.size()),
        std::move(colorClearValues)
    );
}

std::optional<vax::vk::RenderPassDescriptor>
RenderPassDescriptorBuilder::buildShadowSun(VkFormat imageFormat, bool allowComputeUsage) const noexcept {
    VkFormat depthFormat = findDepthFormat(_device.get().vkPhysicalDevice);
    VkAttachmentDescription depthAttachment{
        .format = depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
    };

    VkAttachmentReference depthAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 0,
        .pColorAttachments = nullptr,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };

    std::array<VkSubpassDependency, 2> dependencies;

    VkPipelineStageFlags readingStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if (allowComputeUsage) {
        readingStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }

    constexpr VkPipelineStageFlags depthStageMask =
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

    dependencies[0] = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = readingStages,
        .dstStageMask = depthStageMask,
        .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    dependencies[1] = {
        .srcSubpass = 0,
        .dstSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = depthStageMask,
        .dstStageMask = readingStages,
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    std::array<VkAttachmentDescription, 1> attachments = {depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = static_cast<uint32_t>(dependencies.size()),
        .pDependencies = dependencies.data(),
    };

    VkRenderPass renderPass;
    if (!VK_CHECK(vkCreateRenderPass(_device.get().vkDevice, &renderPassInfo, nullptr, &renderPass))) {
        _logger.error("Failed to create render pass!");
        return std::nullopt;
    }

    return std::make_optional<vax::vk::RenderPassDescriptor>(_device.get(), renderPass, imageFormat, depthFormat, 0);
}