#pragma once

namespace vax::physics {

constexpr static float EARTH_GRAVITY = -9.81f;

struct PhysicsConfig final {
    static PhysicsConfig& defaultConfig() {
        static PhysicsConfig config;
        return config;
    }

    float gravity = EARTH_GRAVITY;
    int substeps = 8;
    int frequency = 200;
};

} // namespace vax::physics