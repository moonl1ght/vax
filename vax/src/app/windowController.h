#pragma once

#include "logger.h"
#include "size.h"
#include "window.h"

namespace vax {
class WindowController final {
  public:
    static constexpr uint32_t maxWindows = 3;

    WindowController() = default;
    ~WindowController() = default;

    WindowController(const WindowController&) = delete;
    WindowController& operator=(const WindowController&) = delete;
    WindowController(WindowController&&) = delete;
    WindowController& operator=(WindowController&&) = delete;

    void setupWindow(uint32_t index, vax::math::SizeUI size, const std::string& name);

    const vax::vk::Window* getWindow(uint32_t index) const { return _windows[index].get(); }

    vax::vk::Window* getWindow(uint32_t index) { return _windows[index].get(); }

    void destroyWindow(uint32_t index);

    void destroyAllWindows();

  private:
    Logger _logger = Logger("WindowController");

    std::array<std::unique_ptr<vax::vk::Window>, maxWindows> _windows;
};
} // namespace vax