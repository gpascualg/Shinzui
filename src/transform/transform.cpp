#include <cstdlib>
#include <cmath>

#include "transform/transform.hpp"

INCL_NOWARN
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
INCL_WARN


Transform::Transform(TimePoint t, glm::vec3 position, glm::vec3 forward, float speed):
    _t0(t),
    _position(position),
    _forward(forward),
    _flags((uint8_t)MovementFlags::STOPPED),
    _speed(speed)
{
    if (std::abs(speed) > glm::epsilon<float>())
    {
        _flags = _flags | (uint8_t)MovementFlags::MOVING;
    }
}

glm::vec3 Transform::position(TimePoint t1)
{
    // If it has stoped, simply return the same point
    if (!isMoving())
    {
        return _position;
    }

    auto time = elapsed(t1);

    if (isRotating())
    {
        auto angle = _initialAngle + _angle * time;
        return _center + glm::vec3 { _radius * std::cos(angle), 0, _radius * std::sin(angle) };
    }

    return _position + _forward * _speed * time;
}

glm::vec3 Transform::forward(TimePoint t1)
{
    if (!isRotating())
    {
        return _forward;
    }
    
    return glm::normalize(glm::rotateY(_forward, _angle * elapsed(t1)));
}

void Transform::startRotation(float angle)
{
    _flags = _flags | (uint8_t)MovementFlags::ROTATING;
    _angle = angle;
    _initialAngle = std::atan2(_position.z, _position.x);
    _center = glm::vec3(_position.x - std::cos(_initialAngle), 0, _position.z - std::sin(_initialAngle));
    recalcParameters();
}

void Transform::setSpeed(float speed)
{
    _speed = speed;
    recalcParameters();
}

void Transform::recalcParameters()
{
    if (isRotating())
    {
        _radius = _speed / _angle;
    }
}
