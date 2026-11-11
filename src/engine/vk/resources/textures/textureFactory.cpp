#include "textureFactory.h"
#include "imageUtils.h"

using namespace vax::vk;
using namespace vax;

std::optional<Texture> TextureFactory::makeDepthTextureDetached(VkFormat format, vax::math::SizeUI size) {
    auto imageResult = vax::vk::createImage(
        _device.get().allocator(),
        size.toExtent3D(),
        format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    if (!imageResult) {
        _logger.error("Failed to create depth texture!");
        return std::nullopt;
    }
    VkDebugUtilsObjectNameInfoEXT nameInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = VK_OBJECT_TYPE_IMAGE,
        .objectHandle = reinterpret_cast<size_t>(imageResult->first),
        .pObjectName = "depth_texture",
    };
    vax::vk::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice(), &nameInfo);
    auto [depthImage, allocation] = imageResult.value();
    auto texture = Texture(
        _device.get(), "depth_texture", depthImage, allocation, size, format, VK_IMAGE_ASPECT_DEPTH_BIT
    );
    return std::make_optional(std::move(texture));
}

std::optional<TextureManager::TextureResource> TextureFactory::makeDepthTexture(VkFormat format, math::SizeUI size) {
    auto texture = makeDepthTextureDetached(format, size);
    if (!texture) {
        return std::nullopt;
    }
    if (_textureManager) {
        auto attached = _textureManager->attach(std::move(*texture));
        if (attached) {
            return std::make_optional(std::move(*attached));
        }
        _logger.error("Failed to attach depth texture to manager");
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<Texture> TextureFactory::makeTextureDetached(const TextureCreateInfo& createInfo) {
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
    if (imageResult) {
        auto [image, allocation] = imageResult.value();
        auto texture = Texture(
            _device.get(),
            createInfo.name,
            image,
            allocation,
            createInfo.size,
            createInfo.format,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
        if (!createInfo.name.empty()) {
            VkDebugUtilsObjectNameInfoEXT nameInfo{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .objectType = VK_OBJECT_TYPE_IMAGE,
                .objectHandle = reinterpret_cast<size_t>(image),
                .pObjectName = createInfo.name.c_str(),
            };
            vax::vk::pfnSetDebugUtilsObjectNameEXT(_device.get().vkDevice(), &nameInfo);
        }
        texture.loadImageView(createInfo.viewType, createInfo.numLayers, createInfo.numMips);
        return std::make_optional(std::move(texture));
    }
    _logger.error("Failed to create texture!");
    return std::nullopt;
}

std::optional<TextureManager::TextureResource> TextureFactory::makeTexture(const TextureCreateInfo& createInfo) {
    auto texture = makeTextureDetached(createInfo);
    if (!texture) {
        return std::nullopt;
    }
    if (_textureManager) {
        auto attached = _textureManager->attach(std::move(*texture));
        if (attached) {
            return std::make_optional(std::move(*attached));
        }
        _logger.error("Failed to attach texture to manager");
        return std::nullopt;
    }
    return std::nullopt;
}