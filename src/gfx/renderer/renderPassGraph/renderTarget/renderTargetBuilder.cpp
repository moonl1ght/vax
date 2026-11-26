#include "renderTargetBuilder.h"
#include "renderDestinationBuilder.h"
#include "renderPassDescriptorBuilder.h"

using namespace vax;
using namespace vax::gfx;
using namespace vax::rhi;

std::unique_ptr<RenderTarget> RenderTargetBuilder::buildForEditor(Swapchain& swapchain) {
    auto descriptorBuilder = RenderPassDescriptorBuilder(_device.get());
    auto mainDescriptor = descriptorBuilder.buildOffscreen(swapchain.swapchainImageFormat, false);
    auto shadowSunDescriptor = descriptorBuilder.buildShadowSun(swapchain.swapchainImageFormat, false);
    auto swapchainDescriptor = descriptorBuilder.buildSwapchain(swapchain.swapchainImageFormat);
    if (!mainDescriptor || !shadowSunDescriptor || !swapchainDescriptor) {
        _logger.error("Failed to create editor render pass descriptors!");
        return nullptr;
    }

    auto& queue = _queueManager.get().getMainGraphicsQueue();
    auto destinationBuilder = RenderDestinationBuilder(_device.get(), _descriptorRegistry.get());

    auto mainDestination =
        destinationBuilder.buildMainOffscreen(_commandManager.get(), queue, swapchain.swapchainExtent, *mainDescriptor);
    if (!mainDestination) {
        _logger.error("Failed to create main render destination!");
        return nullptr;
    }
    auto swapchainDestination =
        destinationBuilder.buildSwapchain(_commandManager.get(), queue, swapchain, *swapchainDescriptor);
    if (!swapchainDestination) {
        _logger.error("Failed to create swapchain render destination!");
        return nullptr;
    }
    auto shadowSunDestination = destinationBuilder.buildShadowSunOffscreen(
        _commandManager.get(), queue, swapchain.swapchainExtent, *shadowSunDescriptor
    );
    if (!shadowSunDestination) {
        _logger.error("Failed to create shadow sun render destination!");
        return nullptr;
    }

    std::unordered_map<std::string, std::unique_ptr<RenderDestination>> destinations;

    destinations["main"] = std::move(mainDestination);
    destinations["swapchain"] = std::move(swapchainDestination);
    destinations["shadow_sun"] = std::move(shadowSunDestination);

    std::unordered_map<std::string, std::unique_ptr<RenderPassDescriptor>> descriptors;
    descriptors["main"] = std::move(mainDescriptor);
    descriptors["shadow_sun"] = std::move(shadowSunDescriptor);
    descriptors["swapchain"] = std::move(swapchainDescriptor);

    return std::make_unique<RenderTarget>(std::move(descriptors), std::move(destinations));
}

std::unique_ptr<RenderTarget> RenderTargetBuilder::buildForSimulation(VkFormat imageFormat, VkExtent2D extent) {
    auto descriptorBuilder = RenderPassDescriptorBuilder(_device.get());
    auto mainDescriptor = descriptorBuilder.buildOffscreen(imageFormat, false);
    auto shadowSunDescriptor = descriptorBuilder.buildShadowSun(imageFormat, false);
    if (!mainDescriptor || !shadowSunDescriptor) {
        _logger.error("Failed to create simulation render pass descriptors!");
        return nullptr;
    }

    auto& queue = _queueManager.get().getSecondaryGraphicsQueue();
    auto destinationBuilder = RenderDestinationBuilder(_device.get(), _descriptorRegistry.get());

    auto mainDestination = destinationBuilder.buildMainOffscreen(_commandManager.get(), queue, extent, *mainDescriptor);
    if (!mainDestination) {
        _logger.error("Failed to create simulation main render destination!");
        return nullptr;
    }
    auto shadowSunDestination =
        destinationBuilder.buildShadowSunOffscreen(_commandManager.get(), queue, extent, *shadowSunDescriptor);
    if (!shadowSunDestination) {
        _logger.error("Failed to create simulation shadow sun render destination!");
        return nullptr;
    }

    std::unordered_map<std::string, std::unique_ptr<RenderDestination>> destinations;
    destinations["main"] = std::move(mainDestination);
    destinations["shadow_sun"] = std::move(shadowSunDestination);

    std::unordered_map<std::string, std::unique_ptr<RenderPassDescriptor>> descriptors;
    descriptors["main"] = std::move(mainDescriptor);
    descriptors["shadow_sun"] = std::move(shadowSunDescriptor);

    return std::make_unique<RenderTarget>(std::move(descriptors), std::move(destinations));
}
