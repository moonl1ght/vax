#pragma once

#include <string>

namespace vax::time {
class TimeManager final {
  public:
    static std::string getCurrentDatetimeString();

    TimeManager();
    ~TimeManager();
};
} // namespace vax::time