#include "editorRenderer.h"

using namespace vax;
using namespace vax::gfx;

bool EditorRenderer::render(scene::EditorScene* scene, const time::FrameTime& frameTime) {
    ZoneScopedN("Renderer::render");
    _waitForFence();

    return true;
}