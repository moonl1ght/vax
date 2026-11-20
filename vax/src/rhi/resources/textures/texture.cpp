#include "texture.h"
#include "imageUtils.h"
#include "textureTaskScheduler.h"

using namespace vax::rhi;
using namespace vax;

void Texture::registerTexture() noexcept {
    switch (_registrationType) {
    case RegistrationType::SampledImage:
        _sampledShaderHandle = _descriptorRegistry.get().registerSampledTextures({this}).value().front();
        break;
    case RegistrationType::StorageImage:
        _storageShaderHandle = _descriptorRegistry.get().registerStorageTextures({this}).value().front();
        break;
    case RegistrationType::Both: {
        _sampledShaderHandle = _descriptorRegistry.get().registerSampledTextures({this}).value().front();
        _storageShaderHandle = _descriptorRegistry.get().registerStorageTextures({this}).value().front();
    } break;
    }
}

void Texture::_destroy() {
    _descriptorRegistry.get().releaseSampledTexture(_sampledShaderHandle);
    _descriptorRegistry.get().releaseStorageTexture(_storageShaderHandle);
    if (_imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(_device.get().vkDevice(), _imageView, nullptr);
        _imageView = VK_NULL_HANDLE;
    }
    if (_image != VK_NULL_HANDLE) {
        vmaDestroyImage(_device.get().allocator(), _image, _allocation);
        _allocation = VK_NULL_HANDLE;
        _image = VK_NULL_HANDLE;
    }
    _name.clear();
    _size = math::SizeUI::zero();
    _format = VK_FORMAT_UNDEFINED;
    _aspectMask = VK_IMAGE_ASPECT_NONE;
    _sampledShaderHandle = {};
    _storageShaderHandle = {};
}

void Texture::loadImageView(VkImageViewType viewType, uint32_t layerCount, uint32_t levelCount) {
    _imageView =
        createImageView(_device.get().vkDevice(), _image, _format, _aspectMask, viewType, layerCount, levelCount)
            .value();
}

bool vax::rhi::Texture::isValid() const { return _image != VK_NULL_HANDLE && _allocation != VK_NULL_HANDLE; }

namespace {
VkImageLayout sampledImageLayoutFor(VkImageAspectFlags aspectMask) {
    return (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL
                                                    : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}
} // namespace

std::optional<VkDescriptorImageInfo> Texture::descriptorImageInfo() const {
    if (_imageView == VK_NULL_HANDLE) {
        _logger.error("Image view is not set");
        return std::nullopt;
    }
    return std::make_optional(
        VkDescriptorImageInfo{
        .imageView = _imageView,
        .imageLayout = sampledImageLayoutFor(_aspectMask),
        }
    );
}

void Texture::loadStagingBuffer(vax::rhi::CommandBuffer& commandBuffer) {
    if (!_stagingBuffer.has_value()) {
        return;
    }
    auto taskSchedulerInline = TextureTaskSchedulerInline(_device.get(), commandBuffer);
    taskSchedulerInline.transitionTextureLayout(
        *this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT
    );
    if (_stagingCopyRegions.empty()) {
        taskSchedulerInline.copyBufferToTexture(*_stagingBuffer, *this);
    } else {
        taskSchedulerInline.copyBufferToTexture(*_stagingBuffer, *this, _stagingCopyRegions);
    }
    taskSchedulerInline.transitionTextureLayout(
        *this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT
    );
}