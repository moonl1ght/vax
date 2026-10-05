#pragma once

#include "luna.h"
#include <cstdint>

namespace vax::engine {
using Color = glm::vec4;
namespace ColorPalette {
static constexpr Color Red = {1.0f, 0.0f, 0.0f, 1.0f};
static constexpr Color Green = {0.0f, 1.0f, 0.0f, 1.0f};
static constexpr Color Blue = {0.0f, 0.0f, 1.0f, 1.0f};
static constexpr Color Yellow = {1.0f, 1.0f, 0.0f, 1.0f};
static constexpr Color Purple = {1.0f, 0.0f, 1.0f, 1.0f};
static constexpr Color Orange = {1.0f, 0.5f, 0.0f, 1.0f};
static constexpr Color Brown = {0.5f, 0.25f, 0.0f, 1.0f};
static constexpr Color Gray = {0.5f, 0.5f, 0.5f, 1.0f};
static constexpr Color Black = {0.0f, 0.0f, 0.0f, 1.0f};
static constexpr Color White = {1.0f, 1.0f, 1.0f, 1.0f};
static constexpr Color Clear = {0.0f, 0.0f, 0.0f, 0.0f};

static std::array<std::byte, 4> getHexColor(Color color) {
    return {
        static_cast<std::byte>(color.r * 255),
        static_cast<std::byte>(color.g * 255),
        static_cast<std::byte>(color.b * 255),
        static_cast<std::byte>(color.a * 255)
    };
}
}; // namespace ColorPalette
} // namespace vax::engine