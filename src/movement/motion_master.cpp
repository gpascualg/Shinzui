/* Copyright 2016 Guillem Pascual */

#include "motion_master.hpp"
#include "map_aware_entity.hpp"
#include "map.hpp"
#include "server.hpp"
#include "debug.hpp"

#include <utility>


MotionMaster::MotionMaster(MapAwareEntity* owner) :
    _owner(owner),
    _flags(0),
    _position{0, 0},
    _forward{0, 1},
    _speed(0)
{}
 
void MotionMaster::update(uint64_t elapsed)
{
    if (isRotating())
    {
        _forward = glm::normalize(_forward + _forwardSpeed * (float)elapsed);
    }

    if (isMoving())
    {
        _position += _forward * (_speed * elapsed);

        LOG_ALWAYS("(%.2f, %.2f)", _position.x, _position.y);

        // TODO(gpascualg): This can be throttled, no need to do cell-changer per tick
        Server::get()->map()->onMove(_owner);
    }
}

void MotionMaster::teleport(glm::vec2 to)
{
    _position = to;
    _flags = 0;
}

void MotionMaster::move()
{
    _flags |= (uint8_t)MovementFlags::MOVING;
}

void MotionMaster::stop()
{
    _flags &= ~(uint8_t)MovementFlags::MOVING;
}

void MotionMaster::speed(float speed)
{
    _speed = speed / 1000.0f;
}

void MotionMaster::forward(float speed)
{
    if (speed == 0)
    {
        _flags &= ~(uint8_t)MovementFlags::ROTATING;
    }
    else
    {
        speed = speed / 1000.0f;
        _forwardSpeed = glm::vec2{ std::cos(speed), std::sin(speed) };
        _flags |= (uint8_t)MovementFlags::ROTATING;
    }
}
