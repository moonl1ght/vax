#include "uiEngine.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#undef Status
#include "imgui.h"
#include "luna.h"

using namespace vax::ui;
using namespace vax;

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
    vkCreateDescriptorPool(_vkEngine.get().device->vkDevice, &pool_info, nullptr, &_imguiDescriptorPool);

    ImGui_ImplSDL3_InitForVulkan(_window.get().window);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = _vkEngine.get().instance;
    init_info.PhysicalDevice = _vkEngine.get().device->vkPhysicalDevice;
    init_info.Device = _vkEngine.get().device->vkDevice;
    init_info.QueueFamily = _vkEngine.get().device->getQueueFamilyIndices().graphicsFamily.value();
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
    auto fontPath = std::string(RES_PATH("fonts/FiraCode-Regular.ttf"));
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 28.0f);
    if (font != nullptr) {
        io.FontDefault = font;
    }
}

void UIEngine::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(_vkEngine.get().device->vkDevice, _imguiDescriptorPool, nullptr);
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