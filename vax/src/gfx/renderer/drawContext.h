#pragma once

#include "vulkanCommon.h"
#include <cstdint>
#include "indirectDrawController.h"

namespace vax::gfx {
struct DrawContext {
    vax::vk::CommandBuffer& commandBuffer;
    uint32_t currentFrame = 0;
};
} // namespace vax::gfx
