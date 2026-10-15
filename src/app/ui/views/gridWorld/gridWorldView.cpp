#include "gridWorldView.h"
#include "fileSystem.h"
#include "gridWorldSceneBuilder.h"
#include "imgui.h"
#include "qlConfig.h"
#include "sceneLoader.h"
#include "viewManager.h"

using namespace vax::ui;
using namespace vax::rl;
using namespace vax::vk;
using namespace vax;

GridWorldView::~GridWorldView() {
    if (auto* agentCameraWindow = _windowController.get().getWindow(1)) {
        agentCameraWindow->setWindowWillHideCallback(nullptr);
        agentCameraWindow->hide();
        agentCameraWindow->cleanupSwapchain();
        agentCameraWindow->destroySurface();
        _windowController.get().destroyWindow(1);
    }
}

void GridWorldView::updateUI(const vax::engine::FrameTime& frameTime) {
    _mainThreadRunner.processThreadQueue();
    _showStatus();

    SceneView::updateUI(frameTime);
}

void GridWorldView::_showMenuBarItems() {
    if (_isDemoLoaded) {
        ImGui::BeginDisabled(_isDemoRunning);
        if (ImGui::MenuItem("Start demo")) {
            _startDemo();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Change start position")) {
            _changeStartPosition();
        }
        ImGui::EndDisabled();
        return;
    }
    ImGui::BeginDisabled(_isTrainingRunning);
    if (ImGui::MenuItem("Load demo")) {
        _toggleDemo();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Reinit grid")) {
        _reinitGrid();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Train")) {
        _startTraining();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Agent camera", nullptr, _isAgentCameraShown, !_isAgentCameraShown)) {
        _showAgentCamera();
    }
    ImGui::EndDisabled();
}

void GridWorldView::_onClose() {
    if (_isDemoLoaded) {
        _toggleDemo();
    } else {
        DebugView::_onClose();
    }
}

void GridWorldView::load(Engine& engine, InputController& inputController) {
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
    auto sceneWorldPair = sceneLoader.load(gridWorldSceneDescriptor);
    _controller =
        std::make_unique<vax::rl::GWSceneController>(std::move(sceneWorldPair.first), std::move(sceneWorldPair.second));

    _controller->load(gridWorldSceneDescriptor);
    _controller->getScene().resize();

    _gridWorld->linkSceneController(_controller.get());
    inputController.addObserver(&_controller->getScene());
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

void GridWorldView::_showStatus() {
    const char* status = nullptr;
    if (_isDemoLoaded) {
        status = _isDemoRunning ? "Demo running" : "Demo loaded";
    } else if (_isTrainingRunning) {
        status = _trainingStatus.c_str();
    } else if (_isTrainingCompleted) {
        status = "Training completed";
    }
    if (status == nullptr) {
        return;
    }
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 10.0f, mainViewport->WorkPos.y + 10.0f));
    ImGui::SetNextWindowBgAlpha(0.35f);
    constexpr ImGuiWindowFlags overlayFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                              ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                              ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs;
    if (_beginInMainWindow("Status", overlayFlags)) {
        ImGui::TextUnformatted(status);
    }
    ImGui::End();
}

void GridWorldView::_reinitGrid() { _gridWorld->reinitWorld(); }

void GridWorldView::_startDemo() {
    _isDemoRunning = true;
    _gridWorld->startDemo([this]() { _isDemoRunning = false; });
}

void GridWorldView::_changeStartPosition() { _gridWorld->changeAgentStartPosition(); }

void GridWorldView::_showAgentCamera() {
    if (_windowController.get().getWindow(1) != nullptr) {
        _windowController.get().getWindow(1)->show();
    } else {
        _windowController.get().setupWindow(1, vax::math::SizeUI{640, 480}, "Agent camera");
        _windowController.get().getWindow(1)->load(true, false);
        _windowController.get().getWindow(1)->createSurface(_uiEngine.get().engine().instance);
        _windowController.get().getWindow(1)->createSwapchain(
            *_uiEngine.get().engine().device, VK_PRESENT_MODE_MAILBOX_KHR
        );
    }
    _windowController.get().getWindow(1)->setWindowWillHideCallback([this]() {
        _isAgentCameraShown = false;
        _controller->getScene().setShouldDrawSecondaryWindow(false);
    });
    _controller->getScene().setShouldDrawSecondaryWindow(true);
    _isAgentCameraShown = true;
}