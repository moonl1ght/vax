#include "app.h"
#include "notificationCenter.h"
#include "profiler.h"
#include "renderdoc.h"
#include "tensorOp.h"
#include <SDL3/SDL_events.h>
#include <cmath>
#include <cuda_runtime.h>

#include "cudaEnv.h"
#include "menuView.h"
#include "physicsEngine.h"

using namespace vax;

bool App::run() {
    if (!_setup()) {
        return false;
    }
    try {
        _mainLoop();
        _cleanup();
    } catch (const std::exception& e) {
        _logger.error("Failed to run app: {}", e.what());
    }
    return true;
}

bool App::_setup() {
    // _testCuda();
    _testPhysics();
    if (!SDL_Vulkan_LoadLibrary(NULL)) {
        _logger.error("Failed to load Vulkan library: {}", SDL_GetError());
        return false;
    }
    RenderDoc::init();
    vax::NotificationCenter::getInstance().setup();
    _windowController = std::make_unique<WindowController>();
    _windowController->setupWindow(0, vax::math::SizeUI{1920, 1080}, "VAX");
    if (!_windowController->getWindow(0)->load(true, true)) {
        return false;
    }
    _engine = std::make_unique<vk::Engine>(*_windowController);
    _engine->setup();

    _uiEngine = std::make_unique<ui::UIEngine>(*_engine, *_windowController->getWindow(0));

    _renderer = std::make_unique<engine::Renderer>(*_engine, *_uiEngine);
    _renderer->setup();

    _viewBuilder =
        std::make_unique<ui::ViewBuilder>(*_uiEngine, *_windowController, _inputController, *_renderer, *_engine);
    _viewManager = std::make_unique<ui::ViewManager>(*_uiEngine, *_viewBuilder);
    _viewManager->setRootView(_viewBuilder->buildMenuView());

    return true;
}

void App::_cleanup() {
    _logger.info("Cleaning up...");
    vkDeviceWaitIdle(_engine->device->vkDevice);
    _uiEngine->cleanup();

    _renderer = nullptr;
    _uiEngine = nullptr;
    _viewManager = nullptr;
    _viewBuilder = nullptr;

    _engine->cleanup();

    _windowController->destroyAllWindows();
    SDL_Quit();
    _logger.info("Cleanup complete!");
}

void App::_mainLoop() {
    if (_windowController->getWindow(0) == nullptr) {
        throw std::runtime_error("Window not initialized");
    }
    static bool running = true;
    int i = 0;
    SDL_WindowID primaryWindowID = _windowController->getWindow(0)->getWindowID();

    while (running) {
        SDL_Event event;
        auto processEvent = [&](SDL_Event& event) {
            _inputController.handleEvent(event);
            _uiEngine->processEvents(event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                return false;
            } else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                if (event.window.windowID == primaryWindowID) {
                    running = false;
                    return false;
                } else if (_windowController->getWindow(1) != nullptr) {
                    if (event.window.windowID == _windowController->getWindow(1)->getWindowID()) {
                        _windowController->getWindow(1)->hide();
                        return true;
                    }
                }
            }
            return true;
        };
        _appMode = _viewManager->getAppMode();
        if (_appMode == AppMode::Continous) {
            while (SDL_PollEvent(&event)) {
                if (!processEvent(event)) {
                    break;
                }
            }
            _loopContinuousUpdate();
        } else if (SDL_WaitEvent(&event)) {
            do {
                processEvent(event);
            } while (SDL_PollEvent(&event));
            _loopByEventUpdate();
        }
        FrameMark;
    }
}

void App::_updateTimestamp() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    static uint32_t frameCount = 0;
    static auto previousTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    _frameTime.timestamp = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    _frameTime.deltaTime =
        std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();
    _frameTime.fps = 1.0f / _frameTime.deltaTime;
    ++frameCount;
    _frameTime.frameCount = frameCount;
    previousTime = currentTime;
}

void App::_loopByEventUpdate() {
    ZoneScoped;

    _viewManager->update(_frameTime);

    _updateTimestamp();

    bool renderResult = false;
    renderResult = _renderer->render(nullptr, _frameTime);

    if (!renderResult) {
        _logger.error("Failed to render scene!");
    }
}

void App::_loopContinuousUpdate() {
    ZoneScoped;

    _viewManager->update(_frameTime);

    _updateTimestamp();
}

void App::_testCuda() const {
    vax::CudaEnv cudaEnv;
    cudaEnv.printInfo();

    _logger.info("===== TensorOpGpu::add =====");

    const std::vector<float> expected = {0.0f, 2.0f, 4.0f, 3.0f, 5.0f, 7.0f};

    auto verify = [this, &expected](const char* label, const math::Tensor& result) {
        bool passed = true;
        for (int i = 0; i < result.totalSize(); ++i) {
            if (std::fabs(result.data()[i] - expected[i]) > 1e-5f) {
                _logger.error(label, " mismatch at ", i, ": got ", result.data()[i], ", expected ", expected[i]);
                passed = false;
            }
        }
        _logger.info(label, ": ", passed ? "PASSED" : "FAILED");
        math::TensorOp::print(result, true);
    };

    auto B = math::Tensor::createArrangeContiguous({3});
    B.allocateGpuMemory();
    if (!B.synchronizeHostToGpu()) {
        _logger.error("Failed to upload B to the device");
        return;
    }

    auto A = math::Tensor::createArrangeContiguous({2, 3});
    A.allocateGpuMemory();
    if (!A.synchronizeHostToGpu()) {
        _logger.error("Failed to upload A to the device");
        return;
    }
    if (!math::TensorOpGpu::add(A, B) || !A.synchronizeGpuToHost()) {
        _logger.error("In-place add failed");
        return;
    }
    verify("in-place add", A);
}

void App::_testPhysics() const {
    vax::physics::PhysicsEngine physicsEngine;
    physicsEngine.test();
}