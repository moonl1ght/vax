#include "textureFactory.h"
#include "imageUtils.h"

using namespace vax::rhi;

std::unique_ptr<Texture> TextureFactory::createDepthTexture(
    VkFormat format, math::SizeUI size, Texture::RegistrationType registrationType
) noexcept {
    auto imageResult = rhi::createImage(
        _device.get().allocator(),
        size.toExtent3D(),
        format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    if (!imageResult) {
        _logger.error("Failed to create depth texture!");
        return nullptr;
    }
    VkDebugUtilsObjectNameInfoEXT nameInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = VK_OBJECT_TYPE_IMAGE,
        .objectHandle = reinterpret_cast<size_t>(imageResult->first),
        .pObjectName = "depth_texture",
    };
    vax::rhi::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice(), &nameInfo);
    auto [depthImage, allocation] = imageResult.value();
    auto texture_id = _uuidGenerator.getUUID();
    auto texture = std::make_unique<Texture>(
        _device.get(),
        _descriptorRegistry.get(),
        registrationType,
        "depth_texture",
        texture_id.str(),
        depthImage,
        allocation,
        size,
        format,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );
    texture->loadImageView(VK_IMAGE_VIEW_TYPE_2D, 1, 1);
    texture->registerTexture();
    return texture;
}

std::unique_ptr<Texture> TextureFactory::createTexture(const TextureCreateInfo& createInfo) noexcept {
    auto imageResult = createImage(
        _device.get().allocator(),
        createInfo.size.toExtent3D(),
        createInfo.format,
        VK_IMAGE_TILING_OPTIMAL,
        createInfo.imageUsageFlags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        createInfo.numLayers,
        createInfo.numMips,
        createInfo.flags
    );
    if (!imageResult) {
        _logger.error("Failed to create texture!");
        return nullptr;
    }
    auto [image, allocation] = imageResult.value();
    if (!createInfo.name.empty()) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_IMAGE,
            .objectHandle = reinterpret_cast<size_t>(image),
            .pObjectName = createInfo.name.c_str(),
        };
        vax::rhi::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice(), &nameInfo);
    }
    auto texture_id = _uuidGenerator.getUUID();
    auto texture = std::make_unique<Texture>(
        _device.get(),
        _descriptorRegistry.get(),
        createInfo.registrationType,
        createInfo.name,
        texture_id.str(),
        image,
        allocation,
        createInfo.size,
        createInfo.format,
        VK_IMAGE_ASPECT_COLOR_BIT
    );
    texture->loadImageView(createInfo.viewType, createInfo.numLayers, createInfo.numMips);
    texture->registerTexture();
    return texture;
}