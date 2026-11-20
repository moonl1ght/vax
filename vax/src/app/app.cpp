#include "app.h"
#include "notificationCenter.h"
#include "profiler.h"
#include "renderdoc.h"
#include "startingView.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_vulkan.h>
#include <cuda_runtime.h>

using namespace vax;

void App::setup() noexcept {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11,wayland,windows,cocoa");
    if (!SDL_Vulkan_LoadLibrary(NULL)) {
        _logger.failure("Failed to load Vulkan library: {}", SDL_GetError());
        return;
    }
    dev::RenderDoc::init();

    vax::NotificationCenter::getInstance().setup();
    _windowController = std::make_unique<WindowController>();
    _windowController->setupWindow(0, vax::math::SizeUI{1920, 1080}, "VAX");
    if (!_windowController->getWindow(0)->load(true, true)) {
        _logger.failure("Failed to load window!");
        return;
    }
    _renderEngine = std::make_unique<gfx::RenderEngine>();
    if (!_renderEngine->setup(*_windowController)) {
        _logger.failure("Failed to setup rendering engine!");
        return;
    }

    _viewBuilder = std::make_unique<ui::ViewBuilder>(
        *_windowController,
        _inputController,
        *_renderEngine
    );
    _viewManager = std::make_unique<ui::ViewManager>(_renderEngine->uiEngine(), *_viewBuilder);
    _viewManager->setRootView(_viewBuilder->buildStartingView());
    _logger.success("App setup complete!");
}

void App::run() noexcept {
    _mainLoop();
    _cleanup();
}

void App::_cleanup() {
    _logger.info("Starting app cleanup...");

    _renderEngine->waitIdle();

    _viewManager = nullptr;
    _viewBuilder = nullptr;

    _renderEngine->cleanup();

    _windowController->destroyAllWindows();
    SDL_Quit();
    _logger.success("App cleanup complete!");
}

void App::_mainLoop() {
    _logger.debug("Starting main loop...");
    if (_windowController->getWindow(0) == nullptr) {
        _logger.failure("Window not initialized");
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
                                         _renderEngine->uiEngine().isSecondaryViewportEvent(event);
            if (!isSecondaryMouseInput) {
                _inputController.handleEvent(event);
            }
            _renderEngine->uiEngine().processEvents(event);
            if (event.type == SDL_EVENT_QUIT) {
                _logger.debug("Quitting app...");
                running = false;
                return false;
            } else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                if (event.window.windowID == primaryWindowID) {
                    _logger.debug("Window close requested...");
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
