#pragma once

#include "gridWorld.h"
#include "gwSceneController.h"
#include "gwTrainingManager.h"
#include "inputController.h"
#include "logger.h"
#include "renderer.h"
#include "sceneView.h"
#include "threadRunner.h"
#include "windowController.h"

namespace vax::gfx {
class RenderEngine;
}

namespace vax::editor {
class GridWorldView final : public SceneView<rl::GWSceneController> {
  public:
    GridWorldView(
        WindowController& windowController,
        gfx::Renderer& renderer,
        gfx::RenderEngine& rendererEngine,
        app::InputController& inputController
    )
        : SceneView(renderer, inputController)
        , _windowController(windowController)
        , _rendererEngine(rendererEngine) {}

    ~GridWorldView();

    GridWorldView(const GridWorldView& other) = delete;
    GridWorldView& operator=(const GridWorldView& other) = delete;
    GridWorldView(GridWorldView&& other) noexcept = delete;
    GridWorldView& operator=(GridWorldView&& other) noexcept = delete;

    void updateUI(const vax::gfx::FrameTime& frameTime) override;

    app::AppMode getAppMode() override { return app::AppMode::Continous; }

  protected:
    bool _load() override;

    const char* _loadingMessage() const override { return "Loading grid world"; }

    void _showMenuBarItems() override;

    bool _isCloseEnabled() const override { return !_isTrainingRunning && !_isDemoRunning; }

    void _onClose() override;

  private:
    dev::Logger _logger = dev::Logger("GridWorldView");
    std::reference_wrapper<WindowController> _windowController;
    std::reference_wrapper<gfx::RenderEngine> _rendererEngine;
    std::unique_ptr<rl::GridWorld> _gridWorld;
    std::unique_ptr<rl::GWTrainingManager> _trainingManager;
    core::ThreadRunner _mainThreadRunner;
    std::string _trainingStatus = "Training not started";
    bool _isTrainingRunning = false;
    bool _isDemoLoaded = false;
    bool _isDemoRunning = false;
    bool _isTrainingCompleted = false;
    bool _isAgentCameraShown = false;

    void _showStatus();

    void _startTraining();

    void _toggleDemo();

    void _reinitGrid();

    void _startDemo();

    void _changeStartPosition();

    void _showAgentCamera();
};
} // namespace vax::editor