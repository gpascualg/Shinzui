/* Copyright 2016 Guillem Pascual */

#include "movement/motion_master.hpp"
#include "map/map_aware_entity.hpp"
#include "map/map.hpp"
#include "movement/movement_generator.hpp"
#include "server/server.hpp"
#include "debug/debug.hpp"

#include <utility>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>


MotionMaster::MotionMaster(MapAwareEntity* owner) :
    _owner(owner),
    _generator(nullptr),
    _flags(0),
    _position{0, 0, 0},
    _forward{0, 0, 1},
    _speed(0)
{}
 
void MotionMaster::update(uint64_t elapsed)
{
    if (_generator)
    {
        auto newPos = _generator->update(_owner, elapsed);
        if (!_generator->hasNext())
        {
            // TODO(gpascualg): Should we delete it?
            delete _generator;
            _generator = nullptr;
        }
        else
        {
            _position = newPos;

            // TODO(gpascualg): This can be throttled, no need to do cell-changer per tick
            Server::get()->map()->onMove(_owner);
        }
    }

    if (isRotating())
    {
        _forward = glm::normalize(glm::rotateY(_forward, _rotationAngle * elapsed));
    }

    if (!_generator && isMoving())
    {
        _position += _forward * (_speed * elapsed);

        // TODO(gpascualg): This can be throttled, no need to do cell-changer per tick
        Server::get()->map()->onMove(_owner);
    }
}

void MotionMaster::teleport(glm::vec3 to)
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
        _rotationAngle = speed / 1000.0f;
        _flags |= (uint8_t)MovementFlags::ROTATING;
    }
}
