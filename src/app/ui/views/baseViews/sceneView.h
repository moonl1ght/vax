#pragma once

#include "debugView.h"
#include "logger.h"
#include "scene.h"

namespace vax::ui {
class SceneView : public DebugView {
  public:
    SceneView(vax::engine::Renderer& renderer)
        : DebugView(renderer) {}

    void updateUI(const vax::engine::FrameTime& frameTime) override { DebugView::updateUI(frameTime); }

    void update(const vax::engine::FrameTime& frameTime) override {
        _frameProfiler->beginFrameZone("frame");
        _frameProfiler->beginFrameZone("frame_update");
        bool renderResult = false;
        vax::engine::SceneUpdateContext sceneUpdateContext{.frameTime = frameTime};
        if (!_isScenePrepared) {
            _renderer.get().setup(&_scene->drawableScene());
            _isScenePrepared = true;
        }
        _scene->drawableScene().update(sceneUpdateContext);
        _frameProfiler->endFrameZone("frame_update");
        DebugView::update(frameTime);
    }

    bool draw(const vax::engine::FrameTime& frameTime) override {
        _frameProfiler->beginFrameZone("draw");
        _renderer.get().prepareForRender(&_scene->drawableScene());
        bool renderResult = _renderer.get().render(&_scene->drawableScene(), frameTime);

        if (!renderResult) {
            _logger.error("Failed to render scene!");
        }
        _frameProfiler->endFrameZone("draw");
        _frameProfiler->endFrameZone("frame");
        return renderResult;
    }

    void viewWillShow() override {
        _renderer.get().linkFrameProfiler(_frameProfiler.get());
        _renderer.get().renderPassGraphMode.set(engine::RenderPassGraphMode::SCENE_GRAPH);
    }

  protected:
    vax::Logger _logger = vax::Logger("SceneView");

    std::unique_ptr<vax::engine::Scene> _scene;

  private:
    bool _isScenePrepared = false;
};
} // namespace vax::ui