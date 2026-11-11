#include "syncObjectsManager.h"
#include "vkEngine.h"
#include "vkUtils.h"

using namespace vax::vk;

bool vax::vk::SyncObjectsManager::setup() {
    _logger.info("Creating synchronization objects...");
    _imageAvailableSemaphores["main"] = std::vector<VkSemaphore>(vax::vk::MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores["main"] = std::vector<VkSemaphore>(vax::vk::MAX_FRAMES_IN_FLIGHT);
    _imageAvailableSemaphores["secondary"] = std::vector<VkSemaphore>(vax::vk::MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores["secondary"] = std::vector<VkSemaphore>(vax::vk::MAX_FRAMES_IN_FLIGHT);
    _inFlightFences = std::vector<VkFence>(vax::vk::MAX_FRAMES_IN_FLIGHT);
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < vax::vk::MAX_FRAMES_IN_FLIGHT; ++i) {
        if (!VK_CHECK(vkCreateSemaphore(
                _device.get().vkDevice(), &semaphoreInfo, nullptr, &_imageAvailableSemaphores["main"][i]
            ))) {
            return false;
        }
        if (!VK_CHECK(vkCreateSemaphore(
                _device.get().vkDevice(), &semaphoreInfo, nullptr, &_renderFinishedSemaphores["main"][i]
            ))) {
            return false;
        }
        if (!VK_CHECK(vkCreateSemaphore(
                _device.get().vkDevice(), &semaphoreInfo, nullptr, &_imageAvailableSemaphores["secondary"][i]
            ))) {
            return false;
        }
        if (!VK_CHECK(vkCreateSemaphore(
                _device.get().vkDevice(), &semaphoreInfo, nullptr, &_renderFinishedSemaphores["secondary"][i]
            ))) {
            return false;
        }
        if (!VK_CHECK(vkCreateFence(_device.get().vkDevice(), &fenceInfo, nullptr, &_inFlightFences[i]))) {
            return false;
        }
    }

    return true;
}

bool vax::vk::SyncObjectsManager::cleanup() {
    for (size_t i = 0; i < _imageAvailableSemaphores.size(); ++i) {
        vkDestroySemaphore(_device.get().vkDevice(), _imageAvailableSemaphores["main"][i], nullptr);
        vkDestroySemaphore(_device.get().vkDevice(), _renderFinishedSemaphores["main"][i], nullptr);
        vkDestroySemaphore(_device.get().vkDevice(), _imageAvailableSemaphores["secondary"][i], nullptr);
        vkDestroySemaphore(_device.get().vkDevice(), _renderFinishedSemaphores["secondary"][i], nullptr);
        vkDestroyFence(_device.get().vkDevice(), _inFlightFences[i], nullptr);
    }
    _imageAvailableSemaphores.clear();
    _renderFinishedSemaphores.clear();
    _inFlightFences.clear();
    return true;
}