#pragma once

#include "commandBuffer.h"
#include "device.h"
#include <vector>

namespace vax::rhi {
class CommandManager final {
  public:

    explicit CommandManager(const vax::rhi::Device& device)
        : _device(device) {};

    CommandManager(const CommandManager& other) = delete;
    CommandManager(CommandManager&& other) noexcept = delete;
    CommandManager& operator=(const CommandManager& other) = delete;
    CommandManager& operator=(CommandManager&& other) noexcept = delete;

    bool setup();

    void cleanup();

    vax::rhi::CommandBuffer createSingleTimeCommandBuffer();

    CommandBuffer& getCommandBuffer(uint32_t index);

  private:
    vax::Logger _logger = vax::Logger("CommandManager");
    std::reference_wrapper<const vax::rhi::Device> _device;
    VkCommandPool _commandPool = VK_NULL_HANDLE;
    std::vector<CommandBuffer> _commandBuffers;

    bool _createCommandPool();
    bool _createCommandBuffer();
};
} // namespace vax::rhi