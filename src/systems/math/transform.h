#pragma once

#include "luna.h"
#include <glm/ext/matrix_float4x4.hpp>

namespace vax::math {
struct Transform final {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f); // in radians
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

    Transform() {};

    Transform(glm::mat4 modelMatrix);

    void updateRotationWithQuaternion(const glm::quat& rotation);

    // accepts rotation in degrees
    void updateRotationInDegrees(const glm::vec3& rotation);

    glm::vec3 getRotationInDegrees() const;

    glm::mat4 getModelMatrix() const;
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