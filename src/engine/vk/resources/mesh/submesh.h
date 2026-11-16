#pragma once

#include "logger.h"
#include "shaderConstants.h"

namespace vax::vk {
struct Submesh final {
    uint32_t indexCount = 0;
    uint32_t firstIndex = 0;
    uint32_t vertexOffset = 0;
    uint32_t materialIndex = NO_MATERIAL_INDEX;

    void debugPrint(const vax::Logger& logger) const;
};
} // namespace vax::vk