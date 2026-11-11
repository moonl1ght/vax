#include "renderDestination.h"

using namespace vax::vk;

void vax::vk::RenderDestination::_destroy() {
    for (auto& framebuffer : framebuffers) {
        vkDestroyFramebuffer(_device.get().vkDevice(), framebuffer, nullptr);
    }
}