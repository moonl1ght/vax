#pragma once

#include <string>

namespace vax {
class TimeManager final {
  public:
    static std::string getCurrentDatetimeString();

    TimeManager();
    ~TimeManager();
};
} // namespace vax