#pragma once

#include "camera.h"

namespace vax::engine {
class Light {
  public:
    Light() : _camera(Camera()), _lightUBOIndex(0) {};
    explicit Light(Camera& camera) : _camera(camera) {};

    ~Light() = default;

    Camera& camera() { return _camera; }

    void setCamera(Camera& camera) { _camera = camera; }

    uint32_t lightUBOIndex() const { return _lightUBOIndex; }

    void setLightUBOIndex(uint32_t lightUBOIndex) { _lightUBOIndex = lightUBOIndex; }

  private:
    Camera _camera;
    uint32_t _lightUBOIndex;
};
} // namespace vax::engine