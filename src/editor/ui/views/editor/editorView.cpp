#include "editorView.h"

using namespace vax;
using namespace vax::editor;

void EditorView::updateUI(const vax::time::FrameTime& frameTime) {
    if (_shouldClose) {
        _shouldClose = false;
        DebugView::_onClose();
        return;
    }
    DebugView::updateUI(frameTime);
    _loadingView->updateUI(frameTime);
}

void EditorView::viewWillShow() {
    DebugView::viewWillShow();
    _isLoading = true;
    _loadingView->setMessage("Loading scene");
    _loadingView->setProgress(std::nullopt);
    _loadingView->setShown(true);
    _editorController->load(_scenePath, [this](bool success) {
        _isLoading = false;
        _loadingView->setShown(false);
        if (_isCancelling) {
            _isCancelling = false;
            _shouldClose = true;
            return;
        }
        _isLoaded = success;
        if (!success) {
            _logger.error("Failed to load scene: ", _scenePath);
        }
    });
}

void EditorView::_onClose() {
    if (_isLoading) {
        _cancelLoadingAndClose();
        return;
    }
    DebugView::_onClose();
}

void EditorView::_cancelLoadingAndClose() {
    if (!_isLoading || _isCancelling) {
        return;
    }
    _isCancelling = true;
    _loadingView->setMessage("Cancelling");
    _editorController->cancelLoading();
}

bool EditorView::draw(const vax::time::FrameTime& frameTime) {
    _frameProfiler->beginFrameZone("frame");
    {
        _frameProfiler->beginFrameZone("frame_update");

        _frameProfiler->endFrameZone("frame_update");
    }

    if (_isLoading) {
        _loadingView->draw(frameTime);
        return true;
    }

    _frameProfiler->beginFrameZone("draw");
    bool renderResult = _editorRenderer.get().render(nullptr, frameTime);
    if (!renderResult) {
        _logger.error("Failed to render frame!");
    }
    _frameProfiler->endFrameZone("draw");

    _frameProfiler->endFrameZone("frame");
    return renderResult;
}