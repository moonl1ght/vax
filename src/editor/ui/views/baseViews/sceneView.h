#pragma once

#include "debugView.h"
#include "inputController.h"
#include "loadingView.h"
#include "logger.h"
#include "scene.h"
#include "vkEngine.h"

namespace vax::editor {

template <typename ControllerType> class SceneView : public DebugView {
  public:
    SceneView(vax::gfx::RenderEngine& renderEngine, app::InputController& inputController)
        : DebugView(renderEngine)
        , _inputController(inputController)
        , _renderEngine(renderEngine) {
        _loadingView = std::make_unique<LoadingView>(renderEngine.renderer());
    }

    void updateUI(const vax::gfx::FrameTime& frameTime) override {
        DebugView::updateUI(frameTime);
        _loadingView->updateUI(frameTime);
    }

    void update(const vax::gfx::FrameTime& frameTime) override {
        _frameProfiler->beginFrameZone("frame");
        _frameProfiler->beginFrameZone("frame_update");
        _updateLoading();
        if (_loadState == LoadState::LOADED) {
            vax::gfx::SceneUpdateContext sceneUpdateContext{.frameTime = frameTime};
            _controller->update(sceneUpdateContext);
        }
        _frameProfiler->endFrameZone("frame_update");
        DebugView::update(frameTime);
    }

    bool draw(const vax::gfx::FrameTime& frameTime) override {
        _frameProfiler->beginFrameZone("draw");
        vax::gfx::Scene* scene = _loadState == LoadState::LOADED ? &_controller->getScene() : nullptr;
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
            _loadState == LoadState::LOADED ? gfx::RenderPassGraphMode::SCENE_GRAPH : gfx::RenderPassGraphMode::UI_ONLY
        );
    }

    bool isLoaded() const noexcept { return _loadState == LoadState::LOADED; }

  protected:
    dev::Logger _logger = dev::Logger("SceneView");

    std::reference_wrapper<app::InputController> _inputController;
    std::reference_wrapper<vax::gfx::RenderEngine> _renderEngine;

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
                _renderer.get().renderPassGraphMode.set(gfx::RenderPassGraphMode::SCENE_GRAPH);
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

} // namespace vax::editor
