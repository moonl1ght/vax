#pragma once

#include <string>

namespace vax::core {
class TimeManager final {
  public:
    static std::string getCurrentDatetimeString();

    TimeManager();
    ~TimeManager();
};
} // namespace vax::core