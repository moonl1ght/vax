#pragma once

#include <glm/glm.hpp>
#include "shaderUniforms.h"
#include "vaxMath.h"

#include <glm/gtc/quaternion.hpp>

namespace vax::engine {
class Camera final {
  public:
    enum class Projection { perspective, orthographic };

    enum class Aim { target, free };

    Camera(
        vax::math::SizeUI viewPortSize = vax::math::SizeUI::zero(),
        Projection projection = Projection::perspective,
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f),
        vax::math::Radian fov = vax::math::radians_from_degrees(60),
        float nearPlane = 0.1,
        float farPlane = 100,
        float viewSize = 1
    )
        : _viewPortSize(viewPortSize)
        , _projection(projection)
        , _position(position)
        , _target(target)
        , _fov(fov)
        , _whAspectRatio(viewPortSize.whRatio())
        , _nearPlane(nearPlane)
        , _farPlane(farPlane)
        , _viewSize(viewSize) {
        _orientation = _orientationFromEulerAngles(rotation);
        _updateOrbitAnglesForPosition();
        _aimAtTarget();
        _updateViewMatrix();
        _updateProjectionMatrix();
    };

    glm::mat4 projectionMatrix();
    glm::mat4 viewMatrix();

    void setViewPortSize(vax::math::SizeUI viewPortSize);

    void setProjection(Projection projection);

    void setPosition(glm::vec3 position);

    void setRotation(glm::vec3 rotation);

    void setDirection(glm::vec3 direction, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));

    void setOrientation(const glm::quat& orientation);

    void setTarget(glm::vec3 target);

    void setFov(vax::math::Radian fov);

    void setNearPlane(float nearPlane);

    void setFarPlane(float farPlane);

    void setViewSize(float viewSize);

    void setRotationSpeed(float rotationSpeed);

    UniformBufferObject getUniformBufferObject();

    void rotateBy(glm::vec2 delta);

    void zoomBy(float delta);

    void setAim(Aim aim) { _aim = aim; }

    Aim aim() const { return _aim; }

    glm::quat orientation() const { return _orientation; }

    glm::vec3 position() const { return _position; }

    glm::vec3 forward() const;

    glm::vec3 right() const;

    glm::vec3 up() const;

  private:
    vax::math::SizeUI _viewPortSize;
    Projection _projection;
    glm::vec3 _position;
    glm::vec3 _target;
    vax::math::Radian _fov;
    float _whAspectRatio;
    float _nearPlane;
    float _farPlane;
    float _viewSize;
    float _rotationSpeed = 0.002f;

    glm::quat _orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    Aim _aim = Aim::target;

    float _orbitYaw = 0.0f;
    float _orbitPitch = 0.0f;

    glm::mat4 _savedProjectionMatrix;
    glm::mat4 _savedViewMatrix;
    bool _isViewDirty = true;
    bool _isProjectionDirty = true;

    static glm::quat _orientationFromForward(glm::vec3 forward, glm::vec3 up);
    static glm::quat _orientationFromEulerAngles(glm::vec3 rotation);
    static glm::vec3 _fallbackUpFor(glm::vec3 direction);

    void _updateViewMatrix();
    void _updateProjectionMatrix();
    void _updateOrbitAnglesForPosition();
    void _aimAtTarget();
    void _orbitBy(glm::vec2 delta);
    void _lookAroundBy(glm::vec2 delta);
};
} // namespace vax::engine