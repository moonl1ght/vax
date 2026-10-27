#pragma once

#include "luna.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace vax::math {
struct Transform final {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

    Transform() {};

    Transform(glm::mat4 modelMatrix);

    void setOrientation(const glm::quat& orientation) noexcept;

    // accepts rotation in degrees
    void setOrientationInDegrees(const glm::vec3& angles) noexcept;

    // in radians
    void setOrientationFromEulerAngles(const glm::vec3& angles) noexcept;

    // in radians
    glm::vec3 getEulerAngles() const noexcept;

    // in degrees
    glm::vec3 getEulerAnglesInDegrees() const noexcept;

    glm::mat4 getModelMatrix() const noexcept;
};

struct TransformMatrixHandle final {
    TransformMatrixHandle() {};

    TransformMatrixHandle(const glm::mat4& modelMatrix)
        : _modelMatrix(modelMatrix) {
        _updateNormalMatrix();
    };

    void updateFromTransform(const Transform& transform);
    void updateModelMatrix(const glm::mat4& modelMatrix);

    glm::mat4 getModelMatrix() const { return _modelMatrix; }
    glm::mat4 getNormalMatrix() const { return _normalMatrix; }

  private:
    glm::mat4 _modelMatrix = glm::mat4(1.0f);
    glm::mat4 _normalMatrix = glm::mat4(1.0f);

    void _updateNormalMatrix();
};
} // namespace vax::math