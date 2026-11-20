#pragma once

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

#include "paths.h"
#include <string>
#include "renderdoc_app.h"

namespace vax {

class RenderDoc {
  public:
    static void init() {
#ifdef _WIN32
        HMODULE mod = GetModuleHandleA("renderdoc.dll");
        if (!mod)
            mod = LoadLibraryA("C:/Program Files/RenderDoc/renderdoc.dll");
        if (!mod)
            return;

        auto getApi = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(mod, "RENDERDOC_GetAPI"));
#elif defined(__linux__)
        auto getApi = reinterpret_cast<pRENDERDOC_GetAPI>(dlsym(RTLD_DEFAULT, "RENDERDOC_GetAPI"));
        if (!getApi) {
            void* mod = dlopen("/opt/renderdoc/lib/librenderdoc.so", RTLD_NOW | RTLD_NOLOAD);
            if (!mod)
                mod = dlopen("/opt/renderdoc/lib/librenderdoc.so", RTLD_NOW);
            if (!mod)
                return;
            getApi = reinterpret_cast<pRENDERDOC_GetAPI>(dlsym(mod, "RENDERDOC_GetAPI"));
        }
#endif
#if defined(_WIN32) || defined(__linux__)
        if (!getApi)
            return;

        RENDERDOC_API_1_6_0* api = nullptr;
        if (getApi(eRENDERDOC_API_Version_1_6_0, reinterpret_cast<void**>(&api)) != 1)
            return;
        _api = api;

        const std::string capturePath = RELATIVE_PATH("captures/capture");
        _api->SetCaptureFilePathTemplate(capturePath.c_str());
        // RENDERDOC_OverlayBits overlayBits =
        //     static_cast<RENDERDOC_OverlayBits>(eRENDERDOC_Overlay_Enabled | eRENDERDOC_Overlay_CaptureList);
        RENDERDOC_OverlayBits overlayBits = eRENDERDOC_Overlay_None;
        _api->MaskOverlayBits(overlayBits, overlayBits);
#endif
    }

    static void triggerCapture() {
        if (_api)
            _api->TriggerCapture();
    }

    static bool isLoaded() { return _api != nullptr; }

  private:
    inline static RENDERDOC_API_1_6_0* _api = nullptr;
};

} // namespace vax
