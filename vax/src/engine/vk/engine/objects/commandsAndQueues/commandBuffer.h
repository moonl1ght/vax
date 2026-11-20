#pragma once

#include "device.h"
#include "logger.h"
#include "pipeline.h"

namespace vax::vk {
class CommandBuffer {
  public:
    VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;

    explicit CommandBuffer(
        const vax::vk::Device& device, const VkCommandPool commandPool, VkCommandBuffer vkCommandBuffer
    )
        : _commandPool(commandPool)
        , _device(device)
        , vkCommandBuffer(vkCommandBuffer) {};

    ~CommandBuffer() {
        if (_isBegun) {
            end();
        }
        if (vkCommandBuffer != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(_device.get().vkDevice(), _commandPool, 1, &vkCommandBuffer);
        }
    }

    CommandBuffer(const CommandBuffer& other) = delete;
    CommandBuffer& operator=(const CommandBuffer& other) = delete;

    CommandBuffer(CommandBuffer&& other) noexcept
        : vkCommandBuffer(other.vkCommandBuffer)
        , _commandPool(other._commandPool)
        , _device(other._device)
        , _isBegun(other._isBegun) {
        other.vkCommandBuffer = VK_NULL_HANDLE;
        other._commandPool = VK_NULL_HANDLE;
        other._isBegun = false;
    }

    CommandBuffer& operator=(CommandBuffer&& other) noexcept {
        if (this != &other) {
            if (_isBegun) {
                end();
            }
            if (vkCommandBuffer != VK_NULL_HANDLE) {
                vkFreeCommandBuffers(_device.get().vkDevice(), _commandPool, 1, &vkCommandBuffer);
            }
            vkCommandBuffer = other.vkCommandBuffer;
            _commandPool = other._commandPool;
            _device = other._device;
            _isBegun = other._isBegun;
            other.vkCommandBuffer = VK_NULL_HANDLE;
            other._commandPool = VK_NULL_HANDLE;
            other._isBegun = false;
        }
        return *this;
    }

    bool begin();
    bool bindPipeline(const Pipeline* pipeline, VkPipelineBindPoint bindPoint);
    bool end();
    void reset();
    void submitAndWait(VkQueue queue);

  private:
    vax::Logger _logger = vax::Logger("CommandBuffer");
    VkCommandPool _commandPool;
    std::reference_wrapper<const vax::vk::Device> _device;
    bool _isBegun = false;
};
} // namespace vax::vk