#pragma once

namespace vax::gfx {
enum class RenderPassGraphMode {
    UI_ONLY,
    SCENE_GRAPH,
    UNKNOWN,
};

struct RenderPassGraphDescriptor final {
    bool withGizmo;
    bool withSecondaryWindow;
};
} // namespace vax::gfx