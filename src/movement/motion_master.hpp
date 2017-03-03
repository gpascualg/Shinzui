/* Copyright 2016 Guillem Pascual */

#pragma once

#include <utility>
#include <glm/glm.hpp>

class MapAwareEntity;


enum class MovementFlags
{
    STOPPED =   0x1,
    IDLE =      0x2,
    MOVING =    0x4,
    ROTATING =  0x8
};


class MotionMaster
{
public:
    explicit MotionMaster(MapAwareEntity* owner);

    void teleport(glm::vec2 to);
    inline const glm::vec2& position() { return _position; }

    void forward(float speed);
    inline const glm::vec2& forward() { return _forward; }

    void speed(float speed);
    inline const float speed() { return _speed; }
    
    void move();
    void stop();

    inline bool isMoving() { return (_flags & (uint8_t)MovementFlags::MOVING) == (uint8_t)MovementFlags::MOVING; }
    inline bool isRotating() { return (_flags & (uint8_t)MovementFlags::ROTATING) == (uint8_t)MovementFlags::ROTATING; }

    void update(uint64_t elapsed);

private:
    MapAwareEntity* _owner;
    uint8_t _flags;

    glm::vec2 _position;
    glm::vec2 _forward;
    glm::vec2 _forwardSpeed;

    float _speed;
};
