/* Copyright 2016 Guillem Pascual */

#include "motion_master.hpp"
#include "map_aware_entity.hpp"
#include "map.hpp"
#include "server.hpp"

#include <utility>


MotionMaster::MotionMaster(MapAwareEntity* owner) :
    _owner(owner),
    _flags(0),
    _position{0, 0},
    _forward{0, 0},
    _speed(0)
{}

void MotionMaster::update(uint64_t elapsed)
{
    if (isMoving())
    {
        _position += _forward * _speed;

        // TODO(gpascualg): This can be throttled, no need to do cell-changer per tick
        Server::get()->map()->onMove(_owner);
    }
}

void MotionMaster::teleport(glm::vec2 to)
{
    _position = to;
    _flags = 0;
}

void MotionMaster::move(glm::vec2 forward)
{
    _forward = forward;
    _flags |= (uint8_t)MovementFlags::MOVING;
}

void MotionMaster::stop()
{
    _flags &= ~(uint8_t)MovementFlags::MOVING;
}

void MotionMaster::speed(float speed)
{
    _speed = speed;
}

void MotionMaster::forward(glm::vec2 forward)
{
    _forward = forward;
}
