#pragma once

#include "appMode.h"
#include "frameTime.h"
#include "logger.h"
#include "uiEngine.h"
#include "viewBuilder.h"
#include "viewNode.h"

namespace vax::ui {
class ViewManager final {
  public:
    ViewManager(UIEngine& uiEngine, ViewBuilder& viewBuilder)
        : _uiEngine(uiEngine)
        , _viewBuilder(viewBuilder) {}

    ~ViewManager() = default;

    ViewManager(const ViewManager& other) = delete;
    ViewManager& operator=(const ViewManager& other) = delete;
    ViewManager(ViewManager&& other) noexcept = delete;
    ViewManager& operator=(ViewManager&& other) noexcept = delete;

    void update(const vax::engine::FrameTime& frameTime);

    void setRootView(std::unique_ptr<View> view);

    void pushView(std::unique_ptr<View> view);

    void popView();

    vax::AppMode getAppMode() const;

    ViewBuilder& viewBuilder() { return _viewBuilder.get(); }

  private:
    vax::Logger _logger = vax::Logger("ViewManager");

    std::reference_wrapper<UIEngine> _uiEngine;
    std::reference_wrapper<ViewBuilder> _viewBuilder;

    std::unique_ptr<ViewNode> _rootNode = nullptr;

    ViewNode* _topViewNode = nullptr; 

    std::unique_ptr<ViewNode> _pendingRootViewNode = nullptr;
    std::unique_ptr<ViewNode> _pendingPushViewNode = nullptr;
    bool _isPendingPop = false;

    bool _isUpdating = false;

    void _swapRootViewNode(std::unique_ptr<ViewNode> viewNode);

    void _pushPendingViewNode(std::unique_ptr<ViewNode> viewNode);
};
} // namespace vax::ui