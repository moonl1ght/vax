#include "queueManager.h"
#include "vkUtils.h"

using namespace vax::vk;

void QueueManager::setup(const Device& device) {
    _logger.info("Setting up queue manager...");
    QueueFamilyIndices indices = device.queueFamilyIndices();
    vkGetDeviceQueue(device.vkDevice(), indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device.vkDevice(), indices.presentFamily.value(), 0, &presentQueue);
}