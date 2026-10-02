#pragma once

#include "buffer.h"
#include "commandBuffer.h"
#include "commandManager.h"
#include "device.h"
#include "luna.h"
#include "texture.h"

namespace vax::vk {
class TextureTaskSchedulerInline final {
  public:
    explicit TextureTaskSchedulerInline(const vax::vk::Device& device, vax::vk::CommandBuffer& commandBuffer)
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

    void copyBufferToTexture(vax::vk::AnyBuffer& buffer, Texture& texture);

    void copyBufferToTexture(
        vax::vk::AnyBuffer& buffer, Texture& texture, const std::vector<VkBufferImageCopy>& copyRegions
    );

  private:
    std::reference_wrapper<const vax::vk::Device> _device;
    std::reference_wrapper<vax::vk::CommandBuffer> _commandBuffer;
};

class TextureTaskScheduler final {
  public:
    explicit TextureTaskScheduler(const vax::vk::Device& device, vax::vk::CommandManager& commandManager)
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

    void copyBufferToTextureAndSubmit(VkQueue submitQueue, vax::vk::AnyBuffer& buffer, Texture& texture);

  private:
    std::reference_wrapper<const vax::vk::Device> _device;
    std::reference_wrapper<vax::vk::CommandManager> _commandManager;
};
}; // namespace vax::vk