#pragma once

#include "buffer.h"
#include "logger.h"
#include "vaxMath.h"
#include "vulkanCommon.h"
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace vax::rhi {
class TextureManager;
class TextureFactory;
} // namespace vax::rhi

namespace vax::rhi {
class Texture final {
  public:
    friend class vax::rhi::TextureManager;
    friend class vax::rhi::TextureFactory;

    Texture(const vax::rhi::Device& device)
        : _device(device) {};

    Texture(
        const vax::rhi::Device& device,
        std::string name,
        VkImage image,
        VmaAllocation allocation,
        vax::math::SizeUI size,
        VkFormat format,
        VkImageAspectFlags aspectMask
    )
        : _device(device)
        , _name(name)
        , _image(image)
        , _allocation(allocation)
        , _size(size)
        , _format(format)
        , _aspectMask(aspectMask) {}

    Texture(const Texture& other) = delete;
    Texture& operator=(Texture& other) = delete;
    Texture(Texture&& other) noexcept = delete;
    Texture& operator=(Texture&& other) noexcept = delete;

    ~Texture() { cleanup(); }

    void cleanup();

    bool isValid() const;

    void loadImageView(VkImageViewType viewType, uint32_t layerCount, uint32_t levelCount);

    const std::string& name() const { return _name; }

    VkImage image() const { return _image; }

    VkImageView imageView() const { return _imageView; }

    std::optional<VkDescriptorImageInfo> descriptorImageInfo() const;

    vax::math::SizeUI size() const { return _size; }

    uint32_t width() const { return _size.width; }

    uint32_t height() const { return _size.height; }

    void setStagingBuffer(Buffer<void> stagingBuffer, std::vector<VkBufferImageCopy> copyRegions = {}) {
        _stagingBuffer = std::move(stagingBuffer);
        _stagingCopyRegions = std::move(copyRegions);
    }

    bool hasStagingBuffer() const { return _stagingBuffer.has_value(); }

    void loadStagingBuffer(vax::rhi::CommandBuffer& commandBuffer);

    void resetStagingBuffer() {
        _stagingBuffer = std::nullopt;
        _stagingCopyRegions.clear();
    }

  private:
    dev::Logger _logger = dev::Logger("Texture");

    std::reference_wrapper<const vax::rhi::Device> _device;

    std::string _name;

    vax::math::SizeUI _size = vax::math::SizeUI::zero();
    VkFormat _format = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_NONE;
    VkImage _image = VK_NULL_HANDLE;
    VkImageView _imageView = VK_NULL_HANDLE;
    VmaAllocation _allocation = VK_NULL_HANDLE;

    std::optional<Buffer<void>> _stagingBuffer = std::nullopt;
    std::vector<VkBufferImageCopy> _stagingCopyRegions;

    void _destroy();
};
} // namespace vax::rhi