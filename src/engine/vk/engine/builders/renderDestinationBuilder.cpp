#include "renderDestinationBuilder.h"
#include "textureFactory.h"
#include "textureTaskScheduler.h"

using namespace vax::vk;
using namespace vax;

std::optional<RenderDestination> RenderDestinationBuilder::buildMainOffscreen(
    CommandManager& commandManager, VkQueue submitQueue, VkExtent2D extent, RenderPassDescriptor& renderPassDescriptor
) const noexcept {
    _logger.info("Building main offscreen render destination...");
    VkFormat depthFormat = renderPassDescriptor.depthFormat;
    auto depthTexture =
        TextureFactory(_device.get()).makeDepthTextureDetached(depthFormat, math::SizeUI(extent));

    if (!depthTexture.has_value()) {
        return std::nullopt;
    }

    auto textureTaskScheduler = TextureTaskScheduler(_device.get(), commandManager);
    textureTaskScheduler.transitionTextureLayoutAndSubmit(
        submitQueue,
        *depthTexture,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );

    depthTexture->loadImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1);

    VkImageUsageFlags colorImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    std::vector<Texture> colorTextures;
    colorTextures.reserve(MAX_FRAMES_IN_FLIGHT);

    std::vector<Texture> maskTextures;
    maskTextures.reserve(MAX_FRAMES_IN_FLIGHT);

    std::vector<VkFramebuffer> framebuffers;
    framebuffers.resize(MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        auto colorTexture = TextureFactory(_device.get())
                                .makeTextureDetached(
                                    TextureFactory::TextureCreateInfo{
                                    .name = "main_offscreen_color_" + std::to_string(i),
                                    .format = renderPassDescriptor.imageFormat,
                                    .size = math::SizeUI(extent),
                                    .imageUsageFlags = colorImageUsage,
                                    }
                                );
        if (!colorTexture.has_value()) {
            _logger.error("Failed to create offscreen color texture!");
            return std::nullopt;
        }
        colorTexture->loadImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1);
        colorTexture->createSampler();

        auto maskTexture = TextureFactory(_device.get())
                               .makeTextureDetached(
                                   TextureFactory::TextureCreateInfo{
                                   .name = "main_offscreen_mask_" + std::to_string(i),
                                   .format = VK_FORMAT_R32_UINT,
                                   .size = math::SizeUI(extent),
                                   .imageUsageFlags = colorImageUsage,
                                   }
                               );
        if (!maskTexture.has_value()) {
            _logger.error("Failed to create offscreen color texture!");
            return std::nullopt;
        }
        maskTexture->loadImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1);

        std::array<VkImageView, 3> attachments = {
            colorTexture->imageView(), maskTexture->imageView(), depthTexture->imageView()
        };
        VkFramebufferCreateInfo framebufferInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPassDescriptor.getVkRenderPass(),
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = extent.width,
            .height = extent.height,
            .layers = 1,
        };
        if (!VK_CHECK(vkCreateFramebuffer(_device.get().vkDevice, &framebufferInfo, nullptr, &framebuffers[i]))) {
            _logger.error("Failed to create offscreen framebuffer!");
            return std::nullopt;
        }

        colorTextures.push_back(std::move(*colorTexture));
        maskTextures.push_back(std::move(*maskTexture));
    }

    return std::make_optional<RenderDestination>(
        _device.get(),
        std::make_unique<Texture>(std::move(*depthTexture)),
        std::move(colorTextures),
        std::move(maskTextures),
        std::move(framebuffers)
    );
}

std::optional<RenderDestination> RenderDestinationBuilder::buildSwapchain(
    CommandManager& commandManager,
    VkQueue submitQueue,
    Swapchain& swapchain,
    RenderPassDescriptor& renderPassDescriptor
) const noexcept {
    _logger.info("Building swapchain render destination...");
    VkFormat depthFormat = renderPassDescriptor.depthFormat;
    auto depthTexture = TextureFactory(_device.get())
                            .makeDepthTextureDetached(depthFormat, math::SizeUI(swapchain.swapchainExtent));

    if (!depthTexture.has_value()) {
        return std::nullopt;
    }

    auto textureTaskScheduler = TextureTaskScheduler(_device.get(), commandManager);
    textureTaskScheduler.transitionTextureLayoutAndSubmit(
        submitQueue,
        *depthTexture,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );

    depthTexture->loadImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1);

    std::vector<VkFramebuffer> swapchainFramebuffers;
    swapchainFramebuffers.resize(swapchain.swapchainImageViews.size());

    for (size_t i = 0; i < swapchain.swapchainImageViews.size(); ++i) {
        std::array<VkImageView, 2> attachments = {swapchain.swapchainImageViews[i], depthTexture->imageView()};

        VkFramebufferCreateInfo framebufferInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPassDescriptor.getVkRenderPass(),
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = swapchain.swapchainExtent.width,
            .height = swapchain.swapchainExtent.height,
            .layers = 1,
        };
        auto result = vkCreateFramebuffer(_device.get().vkDevice, &framebufferInfo, nullptr, &swapchainFramebuffers[i]);
        if (!VK_CHECK(result)) {
            _logger.error("Failed to create framebuffer!");
            return std::nullopt;
        }
    }

    return std::make_optional<RenderDestination>(
        _device.get(),
        std::make_unique<Texture>(std::move(*depthTexture)),
        std::vector<Texture>{},
        std::vector<Texture>{},
        std::move(swapchainFramebuffers)
    );
}

std::optional<RenderDestination> RenderDestinationBuilder::buildShadowSunOffscreen(
    CommandManager& commandManager, VkQueue submitQueue, VkExtent2D extent, RenderPassDescriptor& renderPassDescriptor
) const noexcept {
    _logger.info("Building shadow sun offscreen render destination...");
    VkFormat depthFormat = renderPassDescriptor.depthFormat;
    auto depthTexture =
        TextureFactory(_device.get()).makeDepthTextureDetached(depthFormat, math::SizeUI(extent));

    if (!depthTexture.has_value()) {
        return std::nullopt;
    }

    auto textureTaskScheduler = TextureTaskScheduler(_device.get(), commandManager);
    textureTaskScheduler.transitionTextureLayoutAndSubmit(
        submitQueue,
        *depthTexture,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );

    depthTexture->loadImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1);

    std::vector<VkFramebuffer> framebuffers;
    framebuffers.resize(MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        auto depthAttachmentRef = depthTexture->imageView();

        VkFramebufferCreateInfo framebufferInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPassDescriptor.getVkRenderPass(),
            .attachmentCount = 1,
            .pAttachments = &depthAttachmentRef,
            .width = extent.width,
            .height = extent.height,
            .layers = 1,
        };
        if (!VK_CHECK(vkCreateFramebuffer(_device.get().vkDevice, &framebufferInfo, nullptr, &framebuffers[i]))) {
            _logger.error("Failed to create offscreen framebuffer!");
            return std::nullopt;
        }
    }

    return std::make_optional<RenderDestination>(
        _device.get(),
        std::make_unique<Texture>(std::move(*depthTexture)),
        std::vector<Texture>{},
        std::vector<Texture>{},
        std::move(framebuffers)
    );
}