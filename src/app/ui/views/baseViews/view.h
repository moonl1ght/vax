#pragma once

#pragma once

#include "appMode.h"
#include "frameTime.h"
#include "renderer.h"

namespace vax::ui {
class ViewManager;

class View {
  public:
    View(vax::engine::Renderer& renderer)
        : _renderer(renderer) {};

    virtual ~View() = default;

    virtual void updateUI(const vax::engine::FrameTime& frameTime) = 0;

    virtual void update(const vax::engine::FrameTime& frameTime) = 0;

    virtual bool draw(const vax::engine::FrameTime& frameTime) {
      _renderer.get().prepareForRender(nullptr);
      return _renderer.get().render(nullptr, frameTime);
    }

    virtual void viewWillShow() {
      _renderer.get().renderPassGraphMode.set(engine::RenderPassGraphMode::UI_ONLY);
    }

    virtual vax::AppMode getAppMode() { return vax::AppMode::EventDriven; }

    void linkViewManager(ViewManager* viewManager) { _viewManager = viewManager; }

  protected:
    std::reference_wrapper<vax::engine::Renderer> _renderer;
    ViewManager* _viewManager = nullptr;
};
} // namespace vax::ui