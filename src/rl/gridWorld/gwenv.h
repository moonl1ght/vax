#pragma once

#include "vaxMath.h"
#include <cstdint>
#include <string_view>

namespace vax::rl {
using State = vax::math::Position2DInt;

enum class AgentOrientation : uint8_t {
    EAST = 0,
    NORTH = 1,
    WEST = 2,
    SOUTH = 3,
};

enum class MoveAction : uint8_t {
    NORTH = 0,
    SOUTH = 1,
    EAST = 2,
    WEST = 3,
};

inline const std::string_view moveActionToString(MoveAction action) {
    switch (action) {
    case MoveAction::NORTH:
        return "North";
    case MoveAction::SOUTH:
        return "South";
    case MoveAction::EAST:
        return "East";
    case MoveAction::WEST:
        return "West";
    default:
        return "Unknown";
    }
}

inline constexpr int numMoveActions = 4;
} // namespace vax::rl