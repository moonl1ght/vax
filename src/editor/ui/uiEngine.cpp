#include "uiEngine.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#undef Status
#include "imgui.h"
#include "paths.h"
#include "vulkanCommon.h"
#include <SDL3/SDL.h>
#include <cstdint>
#include <string>

using namespace vax::editor;

void UIEngine::setup(VkRenderPass renderPass) {
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
    };
    vkCreateDescriptorPool(_vkEngine.get().device->vkDevice(), &pool_info, nullptr, &_imguiDescriptorPool);

    ImGui_ImplSDL3_InitForVulkan(_window.get().window);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = _vkEngine.get().instance;
    init_info.PhysicalDevice = _vkEngine.get().device->vkPhysicalDevice();
    init_info.Device = _vkEngine.get().device->vkDevice();
    init_info.QueueFamily = _vkEngine.get().device->queueFamilyIndices().graphicsFamily.value();
    init_info.Queue = _vkEngine.get().queueManager->graphicsQueue;
    init_info.DescriptorPool = _imguiDescriptorPool;
    init_info.RenderPass = renderPass;
    init_info.MinImageCount = 2;
    init_info.ImageCount = static_cast<uint32_t>(
        _vkEngine.get().getWindowController().getWindow(0)->getSwapchain()->swapchainImages.size()
    );
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info);

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    _setupFonts();
    _applyTheme();

    const float dpi = SDL_GetWindowDisplayScale(_window.get().window);
    if (dpi > 0.0f) {
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(dpi);
        style.FontScaleDpi = dpi;
    }
}

void UIEngine::_setupFonts() {
    ImGuiIO& io = ImGui::GetIO();

    const auto uiPath = std::string(RES_PATH("fonts/Inter-Regular.ttf"));
    const auto iconPath = std::string(RES_PATH("fonts/fa-solid-900.ttf"));
    const auto monoPath = std::string(RES_PATH("fonts/FiraCode-Regular.ttf"));

    ImFont* uiFont = io.Fonts->AddFontFromFileTTF(uiPath.c_str(), UI_FONT_SIZE);

    ImFontConfig iconCfg;
    iconCfg.MergeMode = true;
    iconCfg.PixelSnapH = true;
    iconCfg.GlyphMinAdvanceX = UI_FONT_SIZE;
    io.Fonts->AddFontFromFileTTF(iconPath.c_str(), UI_FONT_SIZE * 0.9f, &iconCfg);

    _monoFont = io.Fonts->AddFontFromFileTTF(monoPath.c_str(), MONO_FONT_SIZE);

    if (uiFont != nullptr) {
        io.FontDefault = uiFont;
    }
}

void UIEngine::_applyTheme() {
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowRounding = 6.0f;
    s.ChildRounding = 4.0f;
    s.FrameRounding = 4.0f;
    s.PopupRounding = 4.0f;
    s.GrabRounding = 4.0f;
    s.TabRounding = 4.0f;
    s.ScrollbarRounding = 6.0f;

    s.WindowPadding = {10, 10};
    s.FramePadding = {8, 5};
    s.ItemSpacing = {8, 6};
    s.ItemInnerSpacing = {6, 4};
    s.ScrollbarSize = 12.0f;
    s.GrabMinSize = 10.0f;

    s.WindowBorderSize = 1.0f;
    s.FrameBorderSize = 0.0f;
    s.TabBorderSize = 0.0f;
    s.WindowTitleAlign = {0.5f, 0.5f};

    const ImVec4 bg = {0.10f, 0.10f, 0.11f, 1.00f};
    const ImVec4 panel = {0.14f, 0.14f, 0.16f, 1.00f};
    const ImVec4 frame = {0.18f, 0.18f, 0.20f, 1.00f};
    const ImVec4 hover = {0.24f, 0.24f, 0.27f, 1.00f};
    const ImVec4 border = {0.22f, 0.22f, 0.25f, 1.00f};
    const ImVec4 accent = {0.33f, 0.55f, 0.95f, 1.00f};
    const ImVec4 accentHover = {0.42f, 0.63f, 1.00f, 1.00f};
    const ImVec4 text = {0.90f, 0.90f, 0.92f, 1.00f};
    const ImVec4 textDim = {0.50f, 0.50f, 0.54f, 1.00f};

    ImVec4* c = s.Colors;
    c[ImGuiCol_Text] = text;
    c[ImGuiCol_TextDisabled] = textDim;
    c[ImGuiCol_WindowBg] = bg;
    c[ImGuiCol_ChildBg] = bg;
    c[ImGuiCol_PopupBg] = panel;
    c[ImGuiCol_Border] = border;
    c[ImGuiCol_FrameBg] = frame;
    c[ImGuiCol_FrameBgHovered] = hover;
    c[ImGuiCol_FrameBgActive] = hover;
    c[ImGuiCol_TitleBg] = panel;
    c[ImGuiCol_TitleBgActive] = panel;
    c[ImGuiCol_TitleBgCollapsed] = panel;
    c[ImGuiCol_MenuBarBg] = panel;
    c[ImGuiCol_ScrollbarBg] = bg;
    c[ImGuiCol_ScrollbarGrab] = frame;
    c[ImGuiCol_ScrollbarGrabHovered] = hover;
    c[ImGuiCol_ScrollbarGrabActive] = accent;
    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_SliderGrabActive] = accentHover;
    c[ImGuiCol_Button] = frame;
    c[ImGuiCol_ButtonHovered] = hover;
    c[ImGuiCol_ButtonActive] = accent;
    c[ImGuiCol_Header] = frame;
    c[ImGuiCol_HeaderHovered] = hover;
    c[ImGuiCol_HeaderActive] = accent;
    c[ImGuiCol_Separator] = border;
    c[ImGuiCol_SeparatorHovered] = hover;
    c[ImGuiCol_SeparatorActive] = accent;
    c[ImGuiCol_ResizeGrip] = frame;
    c[ImGuiCol_ResizeGripHovered] = hover;
    c[ImGuiCol_ResizeGripActive] = accent;
    c[ImGuiCol_Tab] = panel;
    c[ImGuiCol_TabHovered] = hover;
    c[ImGuiCol_TabSelected] = frame;
    c[ImGuiCol_TabSelectedOverline] = accent;
    c[ImGuiCol_TabDimmed] = panel;
    c[ImGuiCol_TabDimmedSelected] = frame;
    c[ImGuiCol_PlotLines] = accent;
    c[ImGuiCol_PlotHistogram] = accent;
    c[ImGuiCol_TextSelectedBg] = {accent.x, accent.y, accent.z, 0.35f};
}

void UIEngine::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(_vkEngine.get().device->vkDevice(), _imguiDescriptorPool, nullptr);
}

void UIEngine::processEvents(SDL_Event& event) { ImGui_ImplSDL3_ProcessEvent(&event); }

void UIEngine::updateUiStart() {
    _imguiDrawData = nullptr;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void UIEngine::updateUiEnd() {
    ImGui::Render();
    _imguiDrawData = ImGui::GetDrawData();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

bool UIEngine::isSecondaryViewportEvent(const SDL_Event& event) const {
    SDL_Window* window = SDL_GetWindowFromEvent(&event);
    if (window == nullptr) {
        return false;
    }
    void* platformHandle = reinterpret_cast<void*>(static_cast<intptr_t>(SDL_GetWindowID(window)));
    ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(platformHandle);
    return viewport != nullptr && viewport != ImGui::GetMainViewport();
}

void UIEngine::render(VkCommandBuffer commandBuffer) {
    if (_imguiDrawData == nullptr) {
        return;
    }
    ImGui_ImplVulkan_RenderDrawData(_imguiDrawData, commandBuffer);
}