#pragma once

#include "cameraDescriptor.h"
#include <nlohmann/json.hpp>

namespace vax::engine {

struct GizmoDescriptor final {
    CameraDescriptor cameraDescriptor;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GizmoDescriptor, cameraDescriptor)

} // namespace vax::engine