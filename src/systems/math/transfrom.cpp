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

    glm::quat rotationQuat = glm::quat_cast(rotationMatrix);
    rotation = glm::eulerAngles(rotationQuat);
}

void Transform::updateRotationWithQuaternion(const glm::quat& rotation) { this->rotation = glm::eulerAngles(rotation); }

void Transform::updateRotationInDegrees(const glm::vec3& rotation) {
    this->rotation = glm::vec3(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z));
}

glm::vec3 Transform::getRotationInDegrees() const {
    return glm::vec3(glm::degrees(rotation.x), glm::degrees(rotation.y), glm::degrees(rotation.z));
}

glm::mat4 Transform::getModelMatrix() const {
    glm::quat rotationQuat = glm::quat(rotation);
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMatrix = glm::mat4_cast(rotationQuat);
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