#include "mathUtils.h"
#include "transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace vax::math;

Transform::Transform(glm::mat4 modelMatrix) {
    position = glm::vec3(modelMatrix[3]);

    scale.x = glm::length(glm::vec3(modelMatrix[0]));
    scale.y = glm::length(glm::vec3(modelMatrix[1]));
    scale.z = glm::length(glm::vec3(modelMatrix[2]));

    glm::mat3 rotationMatrix(
        glm::vec3(modelMatrix[0]) / scale.x, glm::vec3(modelMatrix[1]) / scale.y, glm::vec3(modelMatrix[2]) / scale.z
    );

    orientation = glm::quat_cast(rotationMatrix);
}

void Transform::setOrientation(const glm::quat& orientation) noexcept { this->orientation = orientation; }

void Transform::setOrientationInDegrees(const glm::vec3& angles) noexcept {
    this->orientation = glm::quat(glm::vec3(glm::radians(angles.x), glm::radians(angles.y), glm::radians(angles.z)));
}

void Transform::setOrientationFromEulerAngles(const glm::vec3& angles) noexcept {
    this->orientation = glm::quat(angles);
}

glm::vec3 Transform::getEulerAngles() const noexcept { return glm::eulerAngles(orientation); }

glm::vec3 Transform::getEulerAnglesInDegrees() const noexcept {
    auto angles = glm::eulerAngles(orientation);
    return glm::vec3(glm::degrees(angles.x), glm::degrees(angles.y), glm::degrees(angles.z));
}

glm::mat4 Transform::getModelMatrix() const noexcept {
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMatrix = glm::mat4_cast(orientation);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
    return translationMatrix * rotationMatrix * scaleMatrix;
}

void TransformMatrixHandle::updateFromTransform(const Transform& transform) {
    _modelMatrix = transform.getModelMatrix();
    _updateNormalMatrix();
}

void TransformMatrixHandle::updateModelMatrix(const glm::mat4& modelMatrix) {
    _modelMatrix = modelMatrix;
    _updateNormalMatrix();
}

void TransformMatrixHandle::_updateNormalMatrix() {
    glm::mat3 upperLeft = glm::mat3(_modelMatrix);
    _normalMatrix = glm::mat4(glm::transpose(glm::inverse(upperLeft)));
}