#pragma once

#include "gridWorld.h"
#include "gwTrainingManager.h"
#include "inputController.h"
#include "logger.h"
#include "renderer.h"
#include "sceneView.h"
#include "threadRunner.h"
#include "uiEngine.h"
#include "vkEngine.h"
#include "windowController.h"

namespace vax::ui {
class GridWorldView final : public SceneView {
  public:
    GridWorldView(UIEngine& uiEngine, vax::WindowController& windowController, vax::engine::Renderer& renderer)
        : _uiEngine(uiEngine)
        , _windowController(windowController)
        , SceneView(renderer) {}
    ~GridWorldView();

    GridWorldView(const GridWorldView& other) = delete;
    GridWorldView& operator=(const GridWorldView& other) = delete;
    GridWorldView(GridWorldView&& other) noexcept = delete;
    GridWorldView& operator=(GridWorldView&& other) noexcept = delete;

    void update(const vax::engine::FrameTime& frameTime) override;

    void render(const vax::engine::FrameTime& frameTime) override;

    vax::AppMode getAppMode() override { return vax::AppMode::Continous; }

    void load(vax::vk::Engine& engine, vax::InputController& inputController);

  private:
    vax::Logger _logger = vax::Logger("GridWorldView");
    std::reference_wrapper<UIEngine> _uiEngine;
    std::reference_wrapper<vax::WindowController> _windowController;
    std::unique_ptr<vax::rl::GridWorld> _gridWorld;
    std::unique_ptr<vax::rl::GWTrainingManager> _trainingManager;
    vax::core::ThreadRunner _mainThreadRunner;
    std::string _trainingStatus = "Training not started";
    bool _isTrainingRunning = false;
    bool _isDemoLoaded = false;
    bool _isDemoRunning = false;
    bool _isTrainingCompleted = false;
    bool _isRoverCameraShown = false;

    void _startTraining();

    void _toggleDemo();

    void _reinitGrid();

    void _startDemo();

    void _changeStartPosition();

    void _showRoverCamera();
};
} // namespace vax::ui