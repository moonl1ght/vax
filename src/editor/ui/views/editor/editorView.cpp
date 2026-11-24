#include "editorView.h"

using namespace vax;
using namespace vax::editor;

void EditorView::updateUI(const vax::time::FrameTime& frameTime) { DebugView::updateUI(frameTime); }

void EditorView::viewWillShow() {}

bool EditorView::draw(const vax::time::FrameTime& frameTime) {
    _frameProfiler->beginFrameZone("frame");
    _frameProfiler->beginFrameZone("frame_update");
    // _updateLoading();
    // if (_loadState == LoadState::LOADED) {
    //     vax::gfx::SceneUpdateContext sceneUpdateContext{.frameTime = frameTime};
    //     _controller->update(sceneUpdateContext);
    // }
    _frameProfiler->endFrameZone("frame_update");

    _frameProfiler->beginFrameZone("draw");
    // vax::gfx::Scene* scene = _loadState == LoadState::LOADED ? &_controller->getScene() : nullptr;
    // bool renderResult = _editorRenderer.get().render(scene, frameTime);

    // if (!renderResult) {
    //     _logger.error("Failed to render scene!");
    // }
    _frameProfiler->endFrameZone("draw");
    _frameProfiler->endFrameZone("frame");
    return true;
}