#include "movement/movement_generator.hpp"
#include "debug/debug.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"
#include "movement/motion_master.hpp"
#include "server/server.hpp"

#include <random>


RandomMovement::RandomMovement() :
    _hasPoint(false),
    _bezier(nullptr)
{

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
        static std::uniform_real_distribution<> speedDist(2, 10); // rage 0 - 1
        static std::uniform_real_distribution<> negativeDistanceDist(-400.0f, -250.0f); // rage 0 - 1
        static std::uniform_real_distribution<> positiveDistanceDist(250.0f, 400.0f); // rage 0 - 1
        static std::uniform_real_distribution<> positionDist(100.0f, 200.0f); // rage 0 - 1
        static std::uniform_real_distribution<> normal(0, 1);
        static std::bernoulli_distribution coin;

        uint8_t newSpeed = (uint8_t)speedDist(randomEngine);

        auto coinValue = coin(randomEngine);
        auto position = owner->motionMaster()->position();
        auto forward = owner->motionMaster()->forward();

        glm::vec2 start = { position.x, position.z };
        glm::vec2 end = glm::vec2{ start.x + (float)positionDist(randomEngine), start.y + (float)positionDist(randomEngine) };

        _bezier = new DerivativeBezier(
            start,
            start + glm::vec2{ forward.x, forward.z } *(float)positiveDistanceDist(randomEngine), // StartOffset
            end + glm::vec2{ -forward.x * normal(randomEngine), -forward.z * normal(randomEngine) } *(float)positiveDistanceDist(randomEngine),
            end
            );

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

    _t += _bezier->increase(_t, (float)owner->motionMaster()->speed()) * elapsed;
    auto nextPoint = _bezier->next(_t);
    auto forward = glm::normalize(nextPoint - _previous);
    _previous = nextPoint;
    auto oldForward = owner->motionMaster()->forward();

    glm::vec3 forward3{ forward.x, oldForward.y, forward.y };
    owner->motionMaster()->forward(forward3);

    if (_t >= 1)
    {
        _hasPoint = false;
        owner->motionMaster()->stop();

        LOG(LOG_MOVEMENT_GENERATOR, "Movement End");
    }

    return{ nextPoint.x, owner->motionMaster()->position().y, nextPoint.y };
}

bool RandomMovement::hasNext()
{
    return _bezier != nullptr;
}
