#pragma once

#include "cameraConfig.h"
#include <nlohmann/json.hpp>

namespace vax::scene {

struct GizmoConfig final {
    CameraConfig cameraConfig;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GizmoConfig, cameraConfig)

} // namespace vax::scene