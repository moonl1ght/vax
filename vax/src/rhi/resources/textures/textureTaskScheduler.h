#pragma once

#include "buffer.h"
#include "commandBuffer.h"
#include "commandManager.h"
#include "device.h"
#include "vulkanCommon.h"
#include <vector>
#include "texture.h"

namespace vax::rhi {
class TextureTaskSchedulerInline final {
  public:
    explicit TextureTaskSchedulerInline(const vax::rhi::Device& device, vax::rhi::CommandBuffer& commandBuffer)
        : _device(device)
        , _commandBuffer(commandBuffer) {}

    ~TextureTaskSchedulerInline() {}

    TextureTaskSchedulerInline(const TextureTaskSchedulerInline& other) = delete;
    TextureTaskSchedulerInline& operator=(const TextureTaskSchedulerInline& other) = delete;
    TextureTaskSchedulerInline(TextureTaskSchedulerInline&& other) noexcept = delete;
    TextureTaskSchedulerInline& operator=(TextureTaskSchedulerInline&& other) noexcept = delete;

    void transitionTextureLayout(
        Texture& texture, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask
    );

    void copyBufferToTexture(vax::rhi::AnyBuffer& buffer, Texture& texture);

    void copyBufferToTexture(
        vax::rhi::AnyBuffer& buffer, Texture& texture, const std::vector<VkBufferImageCopy>& copyRegions
    );

  private:
    std::reference_wrapper<const vax::rhi::Device> _device;
    std::reference_wrapper<vax::rhi::CommandBuffer> _commandBuffer;
};

class TextureTaskScheduler final {
  public:
    explicit TextureTaskScheduler(const vax::rhi::Device& device, vax::rhi::CommandManager& commandManager)
        : _device(device)
        , _commandManager(commandManager) {}

    ~TextureTaskScheduler() {}

    TextureTaskScheduler(const TextureTaskScheduler& other) = delete;
    TextureTaskScheduler& operator=(const TextureTaskScheduler& other) = delete;
    TextureTaskScheduler(TextureTaskScheduler&& other) noexcept = delete;
    TextureTaskScheduler& operator=(TextureTaskScheduler&& other) noexcept = delete;

    void transitionTextureLayoutAndSubmit(
        VkQueue submitQueue,
        Texture& texture,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkImageAspectFlags aspectMask
    );

    void copyBufferToTextureAndSubmit(VkQueue submitQueue, vax::rhi::AnyBuffer& buffer, Texture& texture);

  private:
    std::reference_wrapper<const vax::rhi::Device> _device;
    std::reference_wrapper<vax::rhi::CommandManager> _commandManager;
};
}; // namespace vax::rhi