#pragma once

#include <cstdint>

namespace vax::gfx {

enum class FrameUniformSlot : uint32_t {
    MAIN_CAMERA = 0,
    SUN_LIGHT = 1,
    AGENT_CAMERA = 2,
    GIZMO = 3,
    SLOTS_COUNT = 4,
};

} // namespace vax::gfx
