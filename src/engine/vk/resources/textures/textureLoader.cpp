#include "textureLoader.h"
#include "imageUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "./deps/stb_image.h"
#include "buffer.h"
#include "glTokens.h"
#include "textureFactory.h"
#include "textureTaskScheduler.h"
#include <ktx.h>

using namespace vax::vk;
using namespace vax;

VkFormat getVkFormatFromGlInternalFormat(uint32_t glInternalFormat) {
    switch (glInternalFormat) {
    // --- Uncompressed Base Formats ---
    case GL_R8:
        return VK_FORMAT_R8_UNORM;
    case GL_RG8:
        return VK_FORMAT_R8G8_UNORM;
    case GL_RGBA8:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case GL_BGRA8:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case GL_SRGB8_ALPHA8:
        return VK_FORMAT_R8G8B8A8_SRGB;

    // --- Floating Point Formats ---
    case GL_R16F:
        return VK_FORMAT_R16_SFLOAT;
    case GL_RG16F:
        return VK_FORMAT_R16G16_SFLOAT;
    case GL_RGBA16F:
        return VK_FORMAT_R16G16B16A16_SFLOAT;

    case GL_R32F:
        return VK_FORMAT_R32_SFLOAT;
    case GL_RG32F:
        return VK_FORMAT_R32G32_SFLOAT;
    case GL_RGBA32F:
        return VK_FORMAT_R32G32B32A32_SFLOAT;

    // --- BC (DXT) Compressed Formats ---
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        return VK_FORMAT_BC2_UNORM_BLOCK;
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        return VK_FORMAT_BC2_SRGB_BLOCK;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        return VK_FORMAT_BC3_UNORM_BLOCK;
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        return VK_FORMAT_BC3_SRGB_BLOCK;

    // --- RGTC (BC4 / BC5) Compressed Formats ---
    case GL_COMPRESSED_RED_RGTC1:
        return VK_FORMAT_BC4_UNORM_BLOCK;
    case GL_COMPRESSED_SIGNED_RED_RGTC1:
        return VK_FORMAT_BC4_SNORM_BLOCK;
    case GL_COMPRESSED_RG_RGTC2:
        return VK_FORMAT_BC5_UNORM_BLOCK;
    case GL_COMPRESSED_SIGNED_RG_RGTC2:
        return VK_FORMAT_BC5_SNORM_BLOCK;

    // --- BPTC (BC6H / BC7) Compressed Formats ---
    case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
        return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
        return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case GL_COMPRESSED_RGBA_BPTC_UNORM:
        return VK_FORMAT_BC7_UNORM_BLOCK;
    case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        return VK_FORMAT_BC7_SRGB_BLOCK;

    // --- ETC2 Compressed Formats ---
    case GL_COMPRESSED_ETC2_RGB8:
        return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case GL_COMPRESSED_ETC2_SRGB8:
        return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
    case GL_COMPRESSED_ETC2_RGBA8:
        return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    case GL_COMPRESSED_ETC2_SRGB8_ALPHA8:
        return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;

    case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
        return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
        return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;

    default:
        return VK_FORMAT_UNDEFINED;
    }
}

std::optional<TextureManager::TextureResource>
TextureLoader::loadTexture(std::string name, std::span<unsigned char> data, CommandBuffer& commandBuffer) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels =
        stbi_load_from_memory(data.data(), data.size(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) {
        _logger.error("Failed to load pixels");
        return std::nullopt;
    }
    return _loadTexture(name, pixels, &commandBuffer, texWidth, texHeight, texChannels);
}

std::optional<TextureManager::TextureResource>
TextureLoader::loadTexture(std::string path, CommandBuffer& commandBuffer) {
    if (path.ends_with(".ktx")) {
        return _loadKTXTexture(path, &commandBuffer);
    }
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) {
        _logger.error("Failed to load pixels");
        return std::nullopt;
    }
    return _loadTexture(path, pixels, &commandBuffer, texWidth, texHeight, texChannels);
}

std::optional<TextureManager::TextureResource>
TextureLoader::_loadKTXTexture(std::string path, CommandBuffer* commandBuffer) {
    ktxTexture* ktxTex = nullptr;
    ktxResult result = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTex);
    if (result != KTX_SUCCESS) {
        _logger.error("Failed to create ktx texture, ktxResult= ", static_cast<int>(result));
        return std::nullopt;
    }

    VkFormat format = VK_FORMAT_UNDEFINED;
    if (ktxTex->classId == ktxTexture2_c) {
        format = static_cast<VkFormat>(reinterpret_cast<ktxTexture2*>(ktxTex)->vkFormat);
    } else {
        format = getVkFormatFromGlInternalFormat(reinterpret_cast<ktxTexture1*>(ktxTex)->glInternalformat);
    }

    if (format == VK_FORMAT_UNDEFINED) {
        _logger.error("Unsupported or unknown format in KTX file: {}", path);
        ktxTexture_Destroy(ktxTex);
        return std::nullopt;
    }

    const uint32_t width = ktxTex->baseWidth;
    const uint32_t height = ktxTex->baseHeight;
    const uint32_t numFaces = ktxTex->numFaces;
    const uint32_t numMips = ktxTex->numLevels;
    const bool isCubemap = (numFaces == 6);

    const ktx_size_t dataSize = ktxTexture_GetDataSize(ktxTex);

    auto stagingBuffer = vk::AnyBuffer::allocateAndFillData(
        _device.get(),
        path + "_texture_staging_buffer",
        ktxTexture_GetData(ktxTex),
        static_cast<VkDeviceSize>(dataSize),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );
    if (!stagingBuffer.has_value()) {
        ktxTexture_Destroy(ktxTex);
        return std::nullopt;
    }

    std::vector<VkBufferImageCopy> copyRegions;
    copyRegions.reserve(numFaces * numMips);
    for (uint32_t mip = 0; mip < numMips; ++mip) {
        for (uint32_t face = 0; face < numFaces; ++face) {
            ktx_size_t offset = 0;
            ktxTexture_GetImageOffset(ktxTex, mip, 0, face, &offset);
            copyRegions.push_back(
                VkBufferImageCopy{
                .bufferOffset = offset,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource =
                    {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = mip,
                    .baseArrayLayer = face,
                    .layerCount = 1,
                    },
                .imageOffset = {0, 0, 0},
                .imageExtent = {std::max(1u, width >> mip), std::max(1u, height >> mip), 1},
                }
            );
        }
    }

    ktxTexture_Destroy(ktxTex);

    auto textureFactory = _textureManager.get().createTextureFactory();
    auto texture = textureFactory.makeTexture(
        TextureFactory::TextureCreateInfo{
        .name = path,
        .format = format,
        .size = math::SizeUI(width, height),
        .imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .viewType = isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
        .numLayers = numFaces,
        .numMips = numMips
        }
    );
    if (!texture.has_value()) {
        stagingBuffer->cleanup();
        return std::nullopt;
    }

    if (commandBuffer != nullptr) {
        auto taskSchedulerInline = TextureTaskSchedulerInline(_device.get(), *commandBuffer);
        taskSchedulerInline.transitionTextureLayout(
            *(texture->second),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
        vkCmdCopyBufferToImage(
            commandBuffer->vkCommandBuffer,
            stagingBuffer->vkBuffer(),
            texture->second->image(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(copyRegions.size()),
            copyRegions.data()
        );
        taskSchedulerInline.transitionTextureLayout(
            *(texture->second),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
        stagingBuffer->cleanup();
    } else {
        texture->second->setStagingBuffer(std::move(*stagingBuffer));
    }
    return texture;
}

std::optional<TextureManager::TextureResource> TextureLoader::_loadTexture(
    std::string name, unsigned char* pixels, CommandBuffer* commandBuffer, int texWidth, int texHeight, int texChannels
) {
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    auto stagingBuffer = vk::AnyBuffer::allocateAndFillData(
        _device.get(),
        name + "_texture_staging_buffer",
        pixels,
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );

    stbi_image_free(pixels);

    if (!stagingBuffer.has_value()) {
        return std::nullopt;
    }

    auto textureFactory = _textureManager.get().createTextureFactory();

    auto texture = textureFactory.makeTexture(
        TextureFactory::TextureCreateInfo{
        .name = name,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .size = math::SizeUI(texWidth, texHeight),
        .imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        }
    );

    if (!texture.has_value()) {
        return std::nullopt;
    }

    if (commandBuffer != nullptr) {
        auto taskSchedulerInline = TextureTaskSchedulerInline(_device.get(), *commandBuffer);
        taskSchedulerInline.transitionTextureLayout(
            *(texture->second),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
        taskSchedulerInline.copyBufferToTexture(*stagingBuffer, *(texture->second));
        taskSchedulerInline.transitionTextureLayout(
            *(texture->second),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT
        );

        stagingBuffer->cleanup();
    } else {
        texture->second->setStagingBuffer(std::move(*stagingBuffer));
    }
    return texture;
}