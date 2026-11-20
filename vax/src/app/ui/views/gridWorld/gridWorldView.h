#pragma once

#include "gridWorld.h"
#include "gwSceneController.h"
#include "gwTrainingManager.h"
#include "inputController.h"
#include "logger.h"
#include "sceneView.h"
#include "threadRunner.h"
#include "windowController.h"

namespace vax::ui {
class GridWorldView final : public SceneView<rl::GWSceneController> {
  public:
    GridWorldView(
        vax::WindowController& windowController,
        vax::gfx::RenderEngine& renderEngine,
        vax::InputController& inputController
    )
        : SceneView(renderEngine, inputController)
        , _windowController(windowController) {}
    ~GridWorldView();

    GridWorldView(const GridWorldView& other) = delete;
    GridWorldView& operator=(const GridWorldView& other) = delete;
    GridWorldView(GridWorldView&& other) noexcept = delete;
    GridWorldView& operator=(GridWorldView&& other) noexcept = delete;

    void updateUI(const vax::gfx::FrameTime& frameTime) override;

    vax::AppMode getAppMode() override { return vax::AppMode::Continous; }

  protected:
    bool _load() override;

    const char* _loadingMessage() const override { return "Loading grid world"; }

    void _showMenuBarItems() override;

    bool _isCloseEnabled() const override { return !_isTrainingRunning && !_isDemoRunning; }

    void _onClose() override;

  private:
    vax::Logger _logger = vax::Logger("GridWorldView");
    std::reference_wrapper<vax::WindowController> _windowController;
    std::unique_ptr<vax::rl::GridWorld> _gridWorld;
    std::unique_ptr<vax::rl::GWTrainingManager> _trainingManager;
    vax::core::ThreadRunner _mainThreadRunner;
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
} // namespace vax::ui