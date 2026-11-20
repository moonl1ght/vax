#pragma once

#include <atomic>

namespace vax {

class NotificationCenter final {
  public:
    static NotificationCenter& getInstance() {
        static NotificationCenter instance;
        return instance;
    }

    NotificationCenter(const NotificationCenter& other) = delete;
    NotificationCenter& operator=(const NotificationCenter& other) = delete;
    NotificationCenter(NotificationCenter&& other) noexcept = delete;
    NotificationCenter& operator=(NotificationCenter&& other) noexcept = delete;

    void setup();
    void wakeUpSDL();

  private:
    NotificationCenter() = default;
    ~NotificationCenter() = default;

    std::atomic<int> _wakeUpSDLEventId = 0;
};

} // namespace vax