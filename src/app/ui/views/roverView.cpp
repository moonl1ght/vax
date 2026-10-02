#include "roverView.h"
#include "fileSystem.h"
#include "imgui.h"
#include "qlConfig.h"
#include "sceneLoader.h"

using namespace vax::ui;
using namespace vax::rl;
using namespace vax::vk;
using namespace vax;

RoverView::~RoverView() {
    if (_windowController.get().getWindow(1) != nullptr) {
        _windowController.get().getWindow(1)->cleanupSwapchain();
        _windowController.get().getWindow(1)->destroySurface();
    }
}

void RoverView::update(const vax::engine::FrameTime& frameTime) {
    _mainThreadRunner.processThreadQueue();
    ImGui::SetNextWindowSize(ImVec2(380, 320), ImGuiCond_FirstUseEver);
    ImGui::Begin("Rover demo");
    if (_isDemoLoaded) {
        if (_isDemoRunning) {
            ImGui::Text("Demo running");
        } else {
            ImGui::Text("Demo loaded");
            if (ImGui::Button("Start demo", ImVec2(-1, 55))) {
                _startDemo();
            }
            if (ImGui::Button("Change start position", ImVec2(-1, 55))) {
                _changeStartPosition();
            }
            if (ImGui::Button("Back", ImVec2(-1, 55))) {
                _toggleDemo();
            }
        }
    } else {
        if (_isTrainingRunning) {
            ImGui::Text("Training running");
            ImGui::Text("%s", _trainingStatus.c_str());
        } else {
            if (ImGui::Button("Load demo", ImVec2(-1, 55))) {
                _toggleDemo();
            }
            if (ImGui::Button("Reinit grid", ImVec2(-1, 55))) {
                _reinitGrid();
            }
            if (ImGui::Button("Train", ImVec2(-1, 55))) {
                _startTraining();
            }
            if (!_isRoverCameraShown) {
                if (ImGui::Button("Show rover camera", ImVec2(-1, 55))) {
                    _showRoverCamera();
                }
            }
            if (_isTrainingCompleted) {
                ImGui::Text("Training completed");
            }
        }
    }
    ImGui::End();

    _statsView->update(frameTime);
}

void RoverView::render(const vax::engine::FrameTime& frameTime) { _drawScene(frameTime); }

void RoverView::load(Engine& engine, InputController& inputController) {
    _renderer.get().linkFrameProfiler(_frameProfiler);
    _gridWorld = std::make_unique<GridWorld>(QLearningConfig{
        .learningRate = 0.1,
        .gamma = 0.9,
        .epsilon = 0.3,
        .episodes = 100,
    });
    _gridWorld->createRandomGrid();

    auto sceneLoader = vax::engine::SceneLoader(engine);
    _drawableScene =
        sceneLoader.load(RELATIVE_PATH("assets/scenes/gridWorld.json"), _gridWorld->getDrawableDescriptor());
    _drawableScene->resize();
    // _drawableScene->loadScene(_gridWorld->getDrawableDescriptor(), engine.queueManager->graphicsQueue);
    _gridWorld->linkSceneGraph(_drawableScene->sceneGraph());
    inputController.addObserver(_drawableScene.get());
    inputController.addObserver(_gridWorld.get());
}

void RoverView::_startTraining() {
    _isTrainingRunning = true;
    _trainingManager = std::make_unique<vax::rl::GWTrainingManager>();
    _trainingManager->setInititialGrid(_gridWorld->getGrid());
    _trainingManager->startTraining(_mainThreadRunner, [this](TrainingStatus trainingStatus) {
        _trainingStatus = trainingStatus.message;
        if (trainingStatus.isCompleted) {
            _trainingManager.reset();
            _isTrainingRunning = false;
            _isTrainingCompleted = true;
        }
    });
}

void RoverView::_toggleDemo() {
    if (_isDemoLoaded) {
        _isDemoLoaded = false;
        return;
    }
    auto trainPath = RELATIVE_PATH("output/qlearning/");
    auto latestFolder = vax::fs::getLatestFolder(trainPath);
    if (latestFolder.has_value()) {
        _gridWorld->load(latestFolder.value());
        _isDemoLoaded = true;
    }
}

void RoverView::_reinitGrid() { _gridWorld->reinitWorld(); }

void RoverView::_startDemo() {
    _isDemoRunning = true;
    _gridWorld->startDemo([this]() { _isDemoRunning = false; });
}

void RoverView::_changeStartPosition() { _gridWorld->changeAgentStartPosition(); }

void RoverView::_showRoverCamera() {
    if (_windowController.get().getWindow(1) != nullptr) {
        _windowController.get().getWindow(1)->show();
    } else {
        _windowController.get().setupWindow(1, vax::math::SizeUI{640, 480}, "Rover camera");
        _windowController.get().getWindow(1)->load(true, false);
        _windowController.get().getWindow(1)->createSurface(_uiEngine.get().engine().instance);
        _windowController.get().getWindow(1)->createSwapchain(
            *_uiEngine.get().engine().device, VK_PRESENT_MODE_MAILBOX_KHR
        );
    }
    _windowController.get().getWindow(1)->setWindowWillHideCallback([this]() {
        _isRoverCameraShown = false;
        _drawableScene->setShouldDrawSecondaryWindow(false);
    });
    _drawableScene->setShouldDrawSecondaryWindow(true);
    _isRoverCameraShown = true;
}

void RoverView::_drawScene(const vax::engine::FrameTime& frameTime) {
    _frameProfiler->beginFrameZone("frame");
    static bool firstTime = true;
    bool renderResult = false;
    vax::engine::SceneUpdateContext sceneUpdateContext{.frameTime = frameTime};
    if (firstTime) {
        _renderer.get().prepare(_drawableScene.get());
        firstTime = false;
    }
    _drawableScene->update(sceneUpdateContext);

    renderResult = _renderer.get().render(_drawableScene.get(), frameTime);

    if (!renderResult) {
        _logger.error("Failed to render scene!");
    }
    _frameProfiler->endFrameZone("frame");
}