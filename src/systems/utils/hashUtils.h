#pragma once

#include <cstddef>
#include <functional>

namespace vax {

template <typename T> inline void hash_combine(size_t& seed, const T& value) noexcept {
    seed ^= std::hash<T>{}(value) + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
}

} // namespace vax
