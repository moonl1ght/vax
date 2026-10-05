#pragma once

#include "scene.h"
#include "gridWorld.h"
#include "gwTrainingManager.h"
#include "inputController.h"
#include "logger.h"
#include "renderer.h"
#include "statsView.h"
#include "threadRunner.h"
#include "uiEngine.h"
#include "view.h"
#include "vkEngine.h"
#include "windowController.h"
#include "frameProfiler.h"

namespace vax::ui {
class RoverView final : public View {
  public:
    RoverView(UIEngine& uiEngine, vax::WindowController& windowController, vax::engine::Renderer& renderer)
        : _uiEngine(uiEngine)
        , _windowController(windowController)
        , View(renderer) {
        _statsView = std::make_unique<StatsView>(renderer);
        _frameProfiler = std::make_shared<vax::FrameProfiler>();
        _statsView->linkFrameProfiler(_frameProfiler);
    }
    ~RoverView();

    RoverView(const RoverView& other) = delete;
    RoverView& operator=(const RoverView& other) = delete;
    RoverView(RoverView&& other) noexcept = delete;
    RoverView& operator=(RoverView&& other) noexcept = delete;

    void update(const vax::engine::FrameTime& frameTime) override;

    void render(const vax::engine::FrameTime& frameTime) override;

    vax::AppMode getAppMode() override { return vax::AppMode::Continous; }

    void load(vax::vk::Engine& engine, vax::InputController& inputController);

  private:
    vax::Logger _logger = vax::Logger("RoverView");
    std::reference_wrapper<UIEngine> _uiEngine;
    std::reference_wrapper<vax::WindowController> _windowController;
    std::unique_ptr<StatsView> _statsView;
    std::unique_ptr<vax::engine::Scene> _scene;
    std::unique_ptr<vax::rl::GridWorld> _gridWorld;
    std::unique_ptr<vax::rl::GWTrainingManager> _trainingManager;
    std::shared_ptr<vax::FrameProfiler> _frameProfiler;
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

    void _drawScene(const vax::engine::FrameTime& frameTime);
};
} // namespace vax::ui