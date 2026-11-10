#include <SDL3/SDL.h>

#include "app.h"
#include "window.h"
#include "profiler.h"
#ifdef _WIN32
#include "renderdoc.h"
#endif

int main() {
#ifdef _WIN32
    vax::RenderDoc::init();
#endif
// #ifdef TRACY_ENABLE
//     while (!tracy::GetProfiler().IsConnected()) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     }
// #endif

    printf("SDL version: %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);

#if defined(__linux__)
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
#endif
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    std::srand(std::time(0));
    vax::App app = vax::App();
    app.run();

    return EXIT_SUCCESS;
}