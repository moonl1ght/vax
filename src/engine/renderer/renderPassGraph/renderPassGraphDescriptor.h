#pragma once

namespace vax::engine {
    enum class RenderPassGraphMode {
        UI_ONLY,
        SCENE_GRAPH,
        UNKNOWN,
    };

    struct RenderPassGraphDescriptor {
        bool withGizmo;
        bool withSecondaryWindow;
    };
}