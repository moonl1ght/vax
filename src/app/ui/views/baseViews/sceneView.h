#pragma once

#include "debugView.h"
#include "logger.h"
#include "scene.h"

namespace vax::ui {
class SceneView : public DebugView {
  public:
    SceneView(vax::engine::Renderer& renderer)
        : DebugView(renderer) {}

    virtual void drawScene(const vax::engine::FrameTime& frameTime) {
        _frameProfiler->beginFrameZone("frame");
        static bool firstTime = true;
        bool renderResult = false;
        vax::engine::SceneUpdateContext sceneUpdateContext{.frameTime = frameTime};
        if (firstTime) {
            _renderer.get().prepare(&_scene->drawableScene());
            firstTime = false;
        }
        _scene->drawableScene().update(sceneUpdateContext);

        renderResult = _renderer.get().render(&_scene->drawableScene(), frameTime);

        if (!renderResult) {
            _logger.error("Failed to render scene!");
        }
        _frameProfiler->endFrameZone("frame");
    }

    void update(const vax::engine::FrameTime& frameTime) override {
        DebugView::update(frameTime);
    }

  protected:
    vax::Logger _logger = vax::Logger("SceneView");

    std::unique_ptr<vax::engine::Scene> _scene;
};
} // namespace vax::ui