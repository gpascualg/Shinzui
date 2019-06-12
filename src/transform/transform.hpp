/* Copyright 2016 Guillem Pascual */

#pragma once

#include <cstdlib>
#include <cmath>

#include "defs/common.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
INCL_WARN


enum class MovementFlags
{
    STOPPED =   0x1,
    IDLE =      0x2,
    MOVING =    0x4,
    ROTATING =  0x8
};

class Transform
{
public:
    Transform(TimePoint t, glm::vec3 position, glm::vec3 forward, float speed);

    inline auto t();
    inline bool isMoving();
    inline bool isRotating();
    inline float speed(TimePoint t);
    inline float rotation(TimePoint t1);
    inline glm::vec2 position2D(TimePoint t);

    glm::vec3 position(TimePoint t1);
    glm::vec3 forward(TimePoint t1);
    void startRotation(float angle);
    void setSpeed(float speed);

private:
    void recalcParameters();
    inline float elapsed(TimePoint t1);

private:
    TimePoint _t0;
    glm::vec3 _position;
    glm::vec3 _forward;
    glm::vec3 _center;
    uint8_t _flags;
    float _speed;
    float _initialAngle;
    float _angle;
    float _radius;
};


auto Transform::t()
{ 
    return _t0; 
}

bool Transform::isMoving()
{ 
    return (_flags & (uint8_t)MovementFlags::MOVING) == (uint8_t)MovementFlags::MOVING; 
}

bool Transform::isRotating()
{ 
    return (_flags & (uint8_t)MovementFlags::ROTATING) == (uint8_t)MovementFlags::ROTATING; 
}

float Transform::speed(TimePoint t)
{
    // TODO(gpascualg): Acceleration?
    return _speed;
}

float Transform::rotation(TimePoint t1)
{
    return _initialAngle + _angle * elapsed(t1);
}

glm::vec2 Transform::position2D(TimePoint t)
{
    glm::vec3 pos = position(t);
    return { pos.x, pos.z };
}

inline float Transform::elapsed(TimePoint t1)
{
    return static_cast<float>((t1 - _t0).count());
}
