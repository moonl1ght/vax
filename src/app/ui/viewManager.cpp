#include "viewManager.h"

using namespace vax::ui;

void ViewManager::setRootView(std::unique_ptr<View> view) {
    if (_isUpdating) {
        _pendingRootViewNode = std::make_unique<ViewNode>(std::move(view));
        return;
    }
    _swapRootViewNode(std::make_unique<ViewNode>(std::move(view)));

    _topViewNode = _rootNode.get();
}

void ViewManager::pushView(std::unique_ptr<View> view) {
    if (_rootNode == nullptr) {
        _logger.error("No root view set!");
        return;
    }
    if (_isUpdating) {
        _pendingPushViewNode = std::make_unique<ViewNode>(std::move(view));
        return;
    }
    auto newViewNode = std::make_unique<ViewNode>(std::move(view));
    newViewNode->parent = _topViewNode;
    newViewNode->view->linkViewManager(this);
    _topViewNode->child = std::move(newViewNode);
    _topViewNode = _topViewNode->child.get();
}

void ViewManager::popView() {
    auto newTopViewNode = _topViewNode->parent;
    if (newTopViewNode == nullptr) {
        _logger.error("No parent view nodeset!");
        return;
    }
    if (_isUpdating) {
        _isPendingPop = true;
        return;
    }
    vkDeviceWaitIdle(_uiEngine.get().engine().device->vkDevice);
    _topViewNode = newTopViewNode;
    _topViewNode->child = nullptr;
    _isPendingPop = false;
}

void ViewManager::update(const vax::engine::FrameTime& frameTime) {
    if (_topViewNode == nullptr) {
        _logger.error("No top view set!");
        return;
    }
    _isUpdating = true;

    _uiEngine.get().updateUiStart();

    _topViewNode->view->update(frameTime);

    _uiEngine.get().updateUiEnd();

    _topViewNode->view->render(frameTime);

    _isUpdating = false;

    if (_pendingRootViewNode) {
        _swapRootViewNode(std::move(_pendingRootViewNode));
    } else if (_pendingPushViewNode) {
        _pushPendingViewNode(std::move(_pendingPushViewNode));
    } else if (_isPendingPop) {
        popView();
    }
}

vax::AppMode ViewManager::getAppMode() const {
    if (_topViewNode == nullptr) {
        _logger.error("No top view set!");
        return vax::AppMode::EventDriven;
    }
    return _topViewNode->view.get()->getAppMode();
}

void ViewManager::_swapRootViewNode(std::unique_ptr<ViewNode> viewNode) {
    if (_rootNode) {
        vkDeviceWaitIdle(_uiEngine.get().engine().device->vkDevice);
    }
    _rootNode = std::move(viewNode);
    _rootNode->view->linkViewManager(this);
    _topViewNode = _rootNode.get();
}

void ViewManager::_pushPendingViewNode(std::unique_ptr<ViewNode> viewNode) {
    viewNode->parent = _topViewNode;
    viewNode->view->linkViewManager(this);
    _topViewNode->child = std::move(viewNode);
    _topViewNode = _topViewNode->child.get();
}