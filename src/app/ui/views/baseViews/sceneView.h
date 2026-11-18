#pragma once

#include "debugView.h"
#include "inputController.h"
#include "loadingView.h"
#include "logger.h"
#include "scene.h"
#include "vkEngine.h"

namespace vax::ui {

template <typename ControllerType> class SceneView : public DebugView {
  public:
    SceneView(vax::rendering::RenderingEngine& renderingEngine, vax::InputController& inputController)
        : DebugView(renderingEngine)
        , _inputController(inputController)
        , _renderingEngine(renderingEngine) {
        _loadingView = std::make_unique<LoadingView>(renderingEngine.renderer());
    }

    void updateUI(const vax::engine::FrameTime& frameTime) override {
        DebugView::updateUI(frameTime);
        _loadingView->updateUI(frameTime);
    }

    void update(const vax::engine::FrameTime& frameTime) override {
        _frameProfiler->beginFrameZone("frame");
        _frameProfiler->beginFrameZone("frame_update");
        _updateLoading();
        if (_loadState == LoadState::LOADED) {
            vax::engine::SceneUpdateContext sceneUpdateContext{.frameTime = frameTime};
            _controller->update(sceneUpdateContext);
        }
        _frameProfiler->endFrameZone("frame_update");
        DebugView::update(frameTime);
    }

    bool draw(const vax::engine::FrameTime& frameTime) override {
        _frameProfiler->beginFrameZone("draw");
        vax::engine::Scene* scene = _loadState == LoadState::LOADED ? &_controller->getScene() : nullptr;
        bool renderResult = _renderer.get().render(scene, frameTime);

        if (!renderResult) {
            _logger.error("Failed to render scene!");
        }
        _frameProfiler->endFrameZone("draw");
        _frameProfiler->endFrameZone("frame");
        return renderResult;
    }

    void viewWillShow() override {
        _renderer.get().linkFrameProfiler(_frameProfiler.get());
        _renderer.get().renderPassGraphMode.set(
            _loadState == LoadState::LOADED ? engine::RenderPassGraphMode::SCENE_GRAPH
                                            : engine::RenderPassGraphMode::UI_ONLY
        );
    }

    bool isLoaded() const noexcept { return _loadState == LoadState::LOADED; }

  protected:
    vax::Logger _logger = vax::Logger("SceneView");

    std::reference_wrapper<vax::InputController> _inputController;
    std::reference_wrapper<vax::rendering::RenderingEngine> _renderingEngine;

    std::unique_ptr<LoadingView> _loadingView;
    std::unique_ptr<ControllerType> _controller;

    virtual bool _load() = 0;

    virtual const char* _loadingMessage() const { return "Loading scene"; }

  private:
    enum class LoadState {
        NOT_STARTED,
        LOADING,
        LOADED,
        FAILED,
    };

    LoadState _loadState = LoadState::NOT_STARTED;

    void _updateLoading() {
        switch (_loadState) {
        case LoadState::NOT_STARTED:
            _loadingView->setMessage(_loadingMessage());
            _loadingView->setProgress(std::nullopt);
            _loadingView->setShown(true);
            _loadState = LoadState::LOADING;
            break;
        case LoadState::LOADING:
            _renderer.get().load();
            if (_load() && _controller) {
                _loadState = LoadState::LOADED;
                _renderer.get().renderPassGraphMode.set(engine::RenderPassGraphMode::SCENE_GRAPH);
            } else {
                _logger.error("Failed to load scene!");
                _loadState = LoadState::FAILED;
            }
            _loadingView->setShown(false);
            break;
        case LoadState::LOADED:
        case LoadState::FAILED:
            break;
        }
    }
};

} // namespace vax::ui
