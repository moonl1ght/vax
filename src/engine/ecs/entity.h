#pragma once

#include <cstdint>
#include <limits>

namespace vax::ecs {
struct Entity final {
    static constexpr uint32_t NullIndex = std::numeric_limits<uint32_t>::max();

    uint32_t index = NullIndex;
    uint32_t generation = 0;

    constexpr bool isNull() const { return index == NullIndex; }

    constexpr bool operator==(const Entity& other) const = default;
};

inline constexpr Entity NullEntity = {};
} // namespace vax::ecs
