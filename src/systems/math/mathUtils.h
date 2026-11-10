#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace vax::math {
using Degree = float;
using Radian = float;

constexpr Radian radians_from_degrees(Degree degrees) { return glm::radians(degrees); }

constexpr Degree degrees_from_radians(Radian radians) { return glm::degrees(radians); }

constexpr glm::mat4 translationMatrix(const glm::vec3& position) {
    glm::mat4 transform = glm::mat4(1.0f);
    transform[3] = transform[3] + glm::vec4(position, 0.0f);
    return transform;
}

inline glm::mat4 eulerAngleXYZRotationMatrix(const glm::vec3& rotation) {
    glm::mat4 rotationM = glm::mat4(1.0f);
    rotationM = glm::rotate(rotationM, rotation.z, glm::vec3(0, 0, 1));
    rotationM = glm::rotate(rotationM, rotation.y, glm::vec3(0, 1, 0));
    rotationM = glm::rotate(rotationM, rotation.x, glm::vec3(1, 0, 0));
    return rotationM;
}

inline void printMatrix(const glm::mat4& matrix) {
    std::cout << "Matrix: " << matrix[0].x << ", " << matrix[0].y << ", " << matrix[0].z << ", " << matrix[0].w
              << std::endl;
    std::cout << "Matrix: " << matrix[1].x << ", " << matrix[1].y << ", " << matrix[1].z << ", " << matrix[1].w
              << std::endl;
    std::cout << "Matrix: " << matrix[2].x << ", " << matrix[2].y << ", " << matrix[2].z << ", " << matrix[2].w
              << std::endl;
    std::cout << "Matrix: " << matrix[3].x << ", " << matrix[3].y << ", " << matrix[3].z << ", " << matrix[3].w
              << std::endl;
}
} // namespace vax::math