#pragma once

#include "appMode.h"
#include "frameTime.h"
#include "uiEngine.h"
#include "view.h"
#include "viewBuilder.h"

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

    vax::AppMode getAppMode() const;

    View& rootView() { return *_rootView; }

    const View& rootView() const { return *_rootView; }

    ViewBuilder& viewBuilder() { return _viewBuilder.get(); }

  private:
    std::reference_wrapper<UIEngine> _uiEngine;
    std::reference_wrapper<ViewBuilder> _viewBuilder;
    std::unique_ptr<View> _rootView = nullptr;
    std::unique_ptr<View> _pendingRootView = nullptr;
    bool _isUpdating = false;

    void _swapRootView(std::unique_ptr<View> view);
};
} // namespace vax::ui