#include "timeManager.h"

#include <chrono>

using namespace vax;

std::string TimeManager::getCurrentDatetimeString() {
    auto now = std::chrono::system_clock::now();
    std::time_t raw_time = std::chrono::system_clock::to_time_t(now);
    std::tm* time_info = std::localtime(&raw_time);
    std::stringstream ss;
    ss << std::put_time(time_info, "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}