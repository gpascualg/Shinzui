/* Copyright 2016 Guillem Pascual */

#pragma once

#include <utility>
#include <glm/glm.hpp>

class MapAwareEntity;


enum class MovementFlags
{
    STOPPED = 0x1,
    IDLE = 0x2,
    MOVING = 0x4
};


class MotionMaster
{
public:
    explicit MotionMaster(MapAwareEntity* owner);

    inline const glm::vec2& position() { return _position; }
    inline const glm::vec2& forward() { return _forward; }
    inline const float speed() { return _speed; }

    void update(uint64_t elapsed);

    inline bool isMoving() { return _flags & (uint8_t)MovementFlags::MOVING; }
    inline void move() { move(_forward); }
    void move(glm::vec2 forward);

    void teleport(glm::vec2 to);

private:
    MapAwareEntity* _owner;
    uint8_t _flags;

    glm::vec2 _position;
    glm::vec2 _forward;
    float _speed;
};
