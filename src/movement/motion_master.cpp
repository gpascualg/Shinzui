/* Copyright 2016 Guillem Pascual */

#include "movement/motion_master.hpp"
#include "map/map_aware_entity.hpp"
#include "map/map.hpp"
#include "movement/movement_generator.hpp"
#include "server/server.hpp"
#include "debug/debug.hpp"
#include "physics/bounding_box.hpp"

#include <utility>

#include "defs/common.hpp"

INCL_NOWARN
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
INCL_WARN


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
		float elapsedAngle = _rotationAngle * elapsed;

        _forward = glm::normalize(glm::rotateY(_forward, elapsedAngle));
		if (auto bb = _owner->boundingBox())
		{
			bb->rotate(elapsedAngle);
		}
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

void MotionMaster::stop(bool cleanup)
{
    _flags &= ~(uint8_t)MovementFlags::MOVING;
    _flags &= ~(uint8_t)MovementFlags::ROTATING;

    if (cleanup && _generator)
    {
        delete _generator;
        _generator = nullptr;
    }
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
