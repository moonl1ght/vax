#include "app.h"
#include "notificationCenter.h"
#include "physicsEngine.h"
#include "profiler.h"
#include "renderdoc.h"
#include "startingView.h"
#include <SDL3/SDL_events.h>
#include <cuda_runtime.h>

using namespace vax;

bool App::run() {
    if (!_setup()) {
        return false;
    }
    _mainLoop();
    _cleanup();
    return true;
}

bool App::_setup() {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11,wayland,windows,cocoa");
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
    _renderer->load();

    _viewBuilder =
        std::make_unique<ui::ViewBuilder>(*_uiEngine, *_windowController, _inputController, *_renderer, *_engine);
    _viewManager = std::make_unique<ui::ViewManager>(*_uiEngine, *_viewBuilder);
    _viewManager->setRootView(_viewBuilder->buildStartingView());

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
        _logger.error("Window not initialized");
        return;
    }
    static bool running = true;
    int i = 0;
    SDL_WindowID primaryWindowID = _windowController->getWindow(0)->getWindowID();

    while (running) {
        SDL_Event event;
        auto processEvent = [&](SDL_Event& event) {
            bool isSecondaryMouseInput = (event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_WHEEL ||
                                          event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) &&
                                         _uiEngine->isSecondaryViewportEvent(event);
            if (!isSecondaryMouseInput) {
                _inputController.handleEvent(event);
            }
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
            _loopUpdate();
        } else if (SDL_WaitEvent(&event)) {
            do {
                processEvent(event);
            } while (SDL_PollEvent(&event));
            _loopUpdate();
        }
        FrameMark;
    }
    return;
}

void App::_updateTimestamp() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    static auto previousTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    _frameTime.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - startTime);
    _frameTime.deltaTimePrecise = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - previousTime);
    _frameTime.deltaTime =
        std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();
    _frameTime.fps = 1.0f / _frameTime.deltaTime;
    ++_frameTime.frameCount;
    previousTime = currentTime;
}

void App::_loopUpdate() {
    ZoneScoped;

    _updateTimestamp();

    auto cycleResult = _viewManager->runCycle(_frameTime);
    if (!cycleResult) {
        _logger.error("Failed to run app cycle!");
    }
}