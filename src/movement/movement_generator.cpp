/* Copyright 2016 Guillem Pascual */

#include "movement/movement_generator.hpp"
#include "debug/debug.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"
#include "movement/motion_master.hpp"
#include "server/server.hpp"
#include "physics/bounding_box.hpp"

#include <random>


MovementGenerator::~MovementGenerator()
{}

RandomMovement::RandomMovement() :
    _hasPoint(false),
    _bezier(nullptr)
{}

RandomMovement::~RandomMovement()
{
    delete _bezier;
}

boost::intrusive_ptr<Packet> RandomMovement::packet()
{
    // TODO(gpascualg): assert hasNext()
    Packet* broadcast = Packet::create(0x0A05);
    *broadcast << (uint32_t)0;
    *broadcast << _bezier->start() << _bezier->startOffset();
    *broadcast << _bezier->end() << _bezier->endOffset();
    *broadcast << _t;
    return broadcast;
}

glm::vec3 RandomMovement::update(MapAwareEntity* owner, float elapsed)
{
    if (!_hasPoint)
    {
        // TODO(gpascualg): Move this out to somewhere else
        static std::default_random_engine randomEngine;
        static std::uniform_real_distribution<> speedDist(2, 10);
        static std::uniform_real_distribution<> negativeDistanceDist(-400.0f, -250.0f);
        static std::uniform_real_distribution<> positiveDistanceDist(250.0f, 400.0f);
        static std::uniform_real_distribution<> positionDist(100.0f, 200.0f);
        static std::uniform_real_distribution<> normal(0, 1);
        static std::bernoulli_distribution coin;

        uint8_t newSpeed = static_cast<uint8_t>(speedDist(randomEngine));

        auto position = owner->motionMaster()->position();
        auto forward = owner->motionMaster()->forward();

        glm::vec2 start = { position.x, position.z };
        glm::vec2 end = glm::vec2 {  // NOLINT(whitespace/braces)
            start.x + static_cast<float>(positionDist(randomEngine)),
            start.y + static_cast<float>(positionDist(randomEngine))
        };  // NOLINT(whitespace/braces)

        _bezier = new DerivativeBezier(
            start,
            start + glm::vec2{ forward.x, forward.z } *    // NOLINT(whitespace/braces)
                static_cast<float>(positiveDistanceDist(randomEngine)),  // StartOffset
            end + glm::vec2{                               // NOLINT(whitespace/braces)
                    -forward.x * normal(randomEngine),     // NOLINT(whitespace/braces)
                    -forward.z * normal(randomEngine) } *  // NOLINT(whitespace/braces)
                static_cast<float>(positiveDistanceDist(randomEngine)),
            end
        );  // NOLINT(whitespace/parens)

        _t = 0;
        _previous = _bezier->start();

        // TODO(gpascualg): Move this to somewhere else
        // HACK(gpascualg): Packet opcode is not known yet!
        Packet* broadcast = Packet::create(0x0A04);
        *broadcast << owner->id() << newSpeed;
        *broadcast << _bezier->start();

        Server::get()->map()->broadcastToSiblings(owner->cell(), broadcast);
        _hasPoint = true;

        // TODO(gpascualg): Move this to somewhere else
        // HACK(gpascualg): Packet opcode is not known yet!
        auto pathPacket = packet();
        *pathPacket << owner->id();

        Server::get()->map()->broadcastToSiblings(owner->cell(), pathPacket);

        // Start movement
        owner->motionMaster()->speed(newSpeed);
        owner->motionMaster()->move();

        /*
        LOG(LOG_MOVEMENT_GENERATOR, "Moving from (%.2f, %.2f) to (%.2f, %.2f)",
        _start.x, _start.y,
        _end.x, _end.y);

        LOG(LOG_MOVEMENT_GENERATOR, "Offsets from (%.2f, %.2f) to (%.2f, %.2f)",
        _startOffset.x, _startOffset.y,
        _endOffset.x, _endOffset.y);
        */
    }

    _t += _bezier->increase(_t, static_cast<float>(owner->motionMaster()->speed())) * elapsed;
    auto nextPoint = _bezier->next(_t);
    auto forward = glm::normalize(nextPoint - _previous);
    _previous = nextPoint;
    auto oldForward = owner->motionMaster()->forward();

    glm::vec3 forward3{ forward.x, oldForward.y, forward.y };
    owner->motionMaster()->forward(forward3);

    if (auto bb = owner->boundingBox())
    {
        float elapsedAngle = atan2(forward.y - oldForward.y, forward.x - oldForward.x);
        bb->rotate(elapsedAngle);
    }

    if (_t >= 1)
    {
        _hasPoint = false;
        owner->motionMaster()->stop();

        LOG(LOG_MOVEMENT_GENERATOR, "Movement End");

        // TODO(gpascualg): Assert this works
        delete _bezier;
        _bezier = nullptr;
    }

    return{ nextPoint.x, owner->motionMaster()->position().y, nextPoint.y };
}

bool RandomMovement::hasNext()
{
    return _bezier != nullptr;
}

Bezier::~Bezier()
{}

