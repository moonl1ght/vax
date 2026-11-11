#include "commandManager.h"
#include "vkEngine.h"
#include "vkUtils.h"

using namespace vax::vk;

bool vax::vk::CommandManager::setup() {
    if (!_createCommandPool())
        return false;
    if (!_createCommandBuffer())
        return false;
    return true;
}

void vax::vk::CommandManager::cleanup() {

    _commandBuffers.clear();

    vkDestroyCommandPool(_device.get().vkDevice(), _commandPool, nullptr);

    _commandPool = VK_NULL_HANDLE;
}

bool vax::vk::CommandManager::_createCommandPool() {
    _logger.info("Creating command pool...");
    QueueFamilyIndices queueFamilyIndices = _device.get().queueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value()
    };

    if (!VK_CHECK(vkCreateCommandPool(_device.get().vkDevice(), &poolInfo, nullptr, &_commandPool))) {
        _logger.error("Failed to create command pool!");
        return false;
    }

    return true;
}

bool vax::vk::CommandManager::_createCommandBuffer() {
    _logger.info("Creating command buffer...");
    std::vector<VkCommandBuffer> commandBuffers;
    commandBuffers.resize(vax::vk::MAX_FRAMES_IN_FLIGHT);
    _commandBuffers.reserve(commandBuffers.size());
    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = (uint32_t)commandBuffers.size()
    };

    if (!VK_CHECK(vkAllocateCommandBuffers(_device.get().vkDevice(), &allocInfo, commandBuffers.data()))) {
        _logger.error("Failed to allocate command buffers!");
        return false;
    }

    for (size_t i = 0; i < commandBuffers.size(); ++i) {
        _commandBuffers.emplace_back(_device.get(), _commandPool, commandBuffers[i]);
    }

    return true;
}

CommandBuffer vax::vk::CommandManager::createSingleTimeCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device.get().vkDevice(), &allocInfo, &commandBuffer);

    return CommandBuffer(_device.get(), _commandPool, commandBuffer);
}

CommandBuffer& vax::vk::CommandManager::getCommandBuffer(uint32_t index) {
    return _commandBuffers[index];
}