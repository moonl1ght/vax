#pragma once

#include "cameraDescriptor.h"
#include <nlohmann/json.hpp>

namespace vax::engine {

struct GizmoDescriptor final {
  public:
    GizmoDescriptor(const CameraDescriptor& cameraDescriptor) : _cameraDescriptor(cameraDescriptor) {}
    ~GizmoDescriptor() = default;

  private:
    CameraDescriptor _cameraDescriptor;
};

} // namespace vax::engine