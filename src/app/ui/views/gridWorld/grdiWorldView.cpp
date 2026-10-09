#include "fileSystem.h"
#include "gridWorldSceneBuilder.h"
#include "gridWorldView.h"
#include "imgui.h"
#include "qlConfig.h"
#include "sceneLoader.h"
#include "viewManager.h"

using namespace vax::ui;
using namespace vax::rl;
using namespace vax::vk;
using namespace vax;

GridWorldView::~GridWorldView() {
    if (_windowController.get().getWindow(1) != nullptr) {
        _windowController.get().getWindow(1)->cleanupSwapchain();
        _windowController.get().getWindow(1)->destroySurface();
    }
}

void GridWorldView::update(const vax::engine::FrameTime& frameTime) {
    _mainThreadRunner.processThreadQueue();
    ImGui::SetNextWindowSize(ImVec2(380, 360), ImGuiCond_FirstUseEver);
    ImGui::Begin("Grid world demo");
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
            if (ImGui::Button("Back", ImVec2(-1, 55))) {
                _viewManager->popView();
            }
            if (_isTrainingCompleted) {
                ImGui::Text("Training completed");
            }
        }
    }
    ImGui::End();

    SceneView::update(frameTime);
}

void GridWorldView::render(const vax::engine::FrameTime& frameTime) { SceneView::drawScene(frameTime); }

void GridWorldView::load(Engine& engine, InputController& inputController) {
    _renderer.get().linkFrameProfiler(_frameProfiler.get());
    _gridWorld = std::make_unique<GridWorld>(QLearningConfig{
        .learningRate = 0.1,
        .gamma = 0.9,
        .epsilon = 0.3,
        .episodes = 100,
    });
    _gridWorld->createRandomGrid();

    auto sceneLoader = vax::engine::SceneLoader(engine);
    auto gridWorldSceneDescriptor = GridWorldSceneBuilder().buildScene(
        _gridWorld->getGrid(), _gridWorld->getDrawableWorldPosition(), _gridWorld->getAgent()
    );
    _scene = sceneLoader.load(gridWorldSceneDescriptor);
    _scene->drawableScene().resize();
    _gridWorld->linkDrawableWorld(dynamic_cast<vax::rl::GWDrawableWorld*>(_scene->drawableScene().drawableWorld()));
    inputController.addObserver(&_scene->drawableScene());
    inputController.addObserver(_gridWorld.get());
}

void GridWorldView::_startTraining() {
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

void GridWorldView::_toggleDemo() {
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

void GridWorldView::_reinitGrid() { _gridWorld->reinitWorld(); }

void GridWorldView::_startDemo() {
    _isDemoRunning = true;
    _gridWorld->startDemo([this]() { _isDemoRunning = false; });
}

void GridWorldView::_changeStartPosition() { _gridWorld->changeAgentStartPosition(); }

void GridWorldView::_showRoverCamera() {
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
        _scene->drawableScene().setShouldDrawSecondaryWindow(false);
    });
    _scene->drawableScene().setShouldDrawSecondaryWindow(true);
    _isRoverCameraShown = true;
}