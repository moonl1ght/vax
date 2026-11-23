#include "renderDestination.h"

using namespace vax::rhi;

void vax::rhi::RenderDestination::_destroy() noexcept {
    for (auto& framebuffer : framebuffers) {
        vkDestroyFramebuffer(_device.get().vkDevice(), framebuffer, nullptr);
    }
}