#include "camera.h"

using namespace vax::engine;

static constexpr float epsilon = 0.00001f;
static constexpr float squaredEpsilon = epsilon * epsilon;
static constexpr float degenerateBasisEpsilon = 1e-12f;
static constexpr float maxOrbitPitch = static_cast<float>(M_PI_2) - 0.001f;
static constexpr glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
static constexpr glm::vec3 localForward = glm::vec3(0.0f, 0.0f, -1.0f);
static constexpr glm::vec3 localRight = glm::vec3(1.0f, 0.0f, 0.0f);
static constexpr glm::vec3 localUp = glm::vec3(0.0f, 1.0f, 0.0f);
static constexpr float maxPitchCosine = 0.9999f;

void Camera::setViewPortSize(vax::math::SizeUI viewPortSize) {
    _viewPortSize = viewPortSize;
    _whAspectRatio = viewPortSize.whRatio();
    _isProjectionDirty = true;
}

void Camera::setProjection(Projection projection) {
    _projection = projection;
    _isProjectionDirty = true;
}

void Camera::setPosition(glm::vec3 position) {
    _position = position;
    if (_aim == Aim::target) {
        _updateOrbitAnglesForPosition();
        _aimAtTarget();
    }
    _isViewDirty = true;
}

void Camera::setRotation(glm::vec3 rotation) { setOrientation(_orientationFromEulerAngles(rotation)); }

void Camera::setDirection(glm::vec3 direction, glm::vec3 up) {
    if (glm::dot(direction, direction) < squaredEpsilon) {
        return;
    }
    setOrientation(_orientationFromForward(direction, up));
}

void Camera::setOrientation(const glm::quat& orientation) {
    _orientation = glm::normalize(orientation);
    _aim = Aim::free;
    _isViewDirty = true;
}

void Camera::setTarget(glm::vec3 target) {
    _target = target;
    _aim = Aim::target;
    _updateOrbitAnglesForPosition();
    _aimAtTarget();
    _isViewDirty = true;
}

void Camera::setFov(vax::math::Radian fov) {
    _fov = fov;
    _isProjectionDirty = true;
}

void Camera::setNearPlane(float nearPlane) {
    _nearPlane = nearPlane;
    _isProjectionDirty = true;
}

void Camera::setFarPlane(float farPlane) {
    _farPlane = farPlane;
    _isProjectionDirty = true;
}

void Camera::setViewSize(float viewSize) {
    _viewSize = viewSize;
    if (_projection == Projection::orthographic) {
        _isProjectionDirty = true;
    }
}

glm::vec3 Camera::forward() const { return _orientation * localForward; }

glm::vec3 Camera::right() const { return _orientation * localRight; }

glm::vec3 Camera::up() const { return _orientation * localUp; }

glm::mat4 Camera::projectionMatrix() {
    if (_isProjectionDirty) {
        _updateProjectionMatrix();
    }
    return _savedProjectionMatrix;
}

glm::mat4 Camera::viewMatrix() {
    if (_isViewDirty) {
        _updateViewMatrix();
    }
    return _savedViewMatrix;
}

glm::quat Camera::_orientationFromForward(glm::vec3 forward, glm::vec3 up) {
    glm::mat3 basis;
    basis[2] = -glm::normalize(forward);
    glm::vec3 right = glm::cross(up, basis[2]);
    if (glm::dot(right, right) < degenerateBasisEpsilon) {
        right = glm::cross(_fallbackUpFor(basis[2]), basis[2]);
    }
    basis[0] = glm::normalize(right);
    basis[1] = glm::cross(basis[2], basis[0]);
    return glm::quat_cast(basis);
}

glm::quat Camera::_orientationFromEulerAngles(glm::vec3 rotation) {
    return glm::quat_cast(glm::mat3(vax::math::eulerAngleXYZRotationMatrix(rotation)));
}

glm::vec3 Camera::_fallbackUpFor(glm::vec3 direction) {
    auto absolute = glm::abs(direction);
    if (absolute.x <= absolute.y && absolute.x <= absolute.z) {
        return glm::vec3(1.0f, 0.0f, 0.0f);
    }
    return absolute.y <= absolute.z ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
}

void Camera::_aimAtTarget() {
    auto forward = _target - _position;
    if (glm::dot(forward, forward) < squaredEpsilon) {
        return;
    }
    _orientation = _orientationFromForward(forward, worldUp);
}

void Camera::_updateViewMatrix() {
    _isViewDirty = false;
    _savedViewMatrix = glm::mat4_cast(glm::conjugate(_orientation)) * glm::translate(glm::mat4(1.0f), -_position);
}

void Camera::_updateProjectionMatrix() {
    _isProjectionDirty = false;
    if (_whAspectRatio == 0) {
        return;
    }
    switch (_projection) {
    case Projection::perspective:
        _savedProjectionMatrix = glm::perspective(
            _fov, static_cast<float>(_whAspectRatio), static_cast<float>(_farPlane), static_cast<float>(_nearPlane)
        );
        break;
    case Projection::orthographic:
        _savedProjectionMatrix = glm::ortho(
            -_viewSize * _whAspectRatio, _viewSize * _whAspectRatio, -_viewSize, _viewSize, _farPlane, _nearPlane
        );
        break;
    }
}

UniformBufferObject Camera::getUniformBufferObject() {
    return {
        .view = viewMatrix(),
        .proj = projectionMatrix(),
        .cameraPosition = glm::vec4(_position, 1.0f),
        .environmentMapIndex = NO_ENVIRONMENT_MAP_INDEX,
    };
}
void Camera::setRotationSpeed(float rotationSpeed) { _rotationSpeed = rotationSpeed; }

void Camera::rotateBy(glm::vec2 delta) {
    if (_aim == Aim::free) {
        _lookAroundBy(delta);
    } else {
        _orbitBy(delta);
    }
    _isViewDirty = true;
}

void Camera::_orbitBy(glm::vec2 delta) {
    _orbitYaw += delta.x * _rotationSpeed;
    _orbitPitch += (-delta.y) * _rotationSpeed;
    _orbitPitch = std::max(-maxOrbitPitch, std::min(_orbitPitch, maxOrbitPitch));
    auto distanceToTarget = glm::distance(_position, _target);
    auto rotationX = glm::rotate(glm::mat4(1.0f), _orbitPitch, glm::vec3(1.0f, 0.0f, 0.0f));
    auto rotationY = glm::rotate(glm::mat4(1.0f), _orbitYaw, glm::vec3(0.0f, -1.0f, 0.0f));
    auto rotateMatrix = rotationY * rotationX;
    auto distanceVector = glm::vec4(0.0f, 0.0f, distanceToTarget, 0.0f);
    auto rotatedVector = rotateMatrix * distanceVector;
    _position = _target + glm::vec3(rotatedVector);
    _aimAtTarget();
}

void Camera::_lookAroundBy(glm::vec2 delta) {
    auto yaw = glm::angleAxis(-delta.x * _rotationSpeed, worldUp);
    auto pitch = glm::angleAxis(-delta.y * _rotationSpeed, right());
    auto rotated = glm::normalize(yaw * pitch * _orientation);
    if (glm::abs(glm::dot(rotated * localForward, worldUp)) > maxPitchCosine) {
        rotated = glm::normalize(yaw * _orientation);
    }
    _orientation = rotated;
}

void Camera::zoomBy(float delta) {
    if (_aim == Aim::free) {
        _position -= delta * forward();
    } else {
        _position += delta * (_position - _target);
        _aimAtTarget();
    }
    _isViewDirty = true;
}

void Camera::_updateOrbitAnglesForPosition() {
    auto offset = _position - _target;
    _orbitYaw = -std::atan2(offset.x, offset.z);
    float groundDistance = std::sqrt(offset.x * offset.x + offset.z * offset.z);
    _orbitPitch = std::atan2(-offset.y, groundDistance);
}