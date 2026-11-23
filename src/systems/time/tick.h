#pragma once

#include <chrono>
#include <cstdint>

namespace vax::time {

template <int Hz> using Tick = std::chrono::duration<int64_t, std::ratio<1, Hz>>;

} // namespace vax::time
