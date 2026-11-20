#include "notificationCenter.h"
#include <SDL3/SDL.h>
#include <cstring>

using namespace vax::app;

void NotificationCenter::setup() noexcept { _wakeUpSDLEventId.store(SDL_RegisterEvents(1)); }

void NotificationCenter::wakeUpSDL() noexcept {
    SDL_Event event;
    std::memset(&event, 0, sizeof(event));
    event.type = _wakeUpSDLEventId.load();
    SDL_PushEvent(&event);
}