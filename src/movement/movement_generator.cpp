#include "movement_generator.hpp"
#include "debug.hpp"
#include "map.hpp"
#include "map_aware_entity.hpp"
#include "motion_master.hpp"
#include "server.hpp"

#include <random>


RandomMovement::RandomMovement():
    _hasPoint(false)
{

}

glm::vec2 RandomMovement::next(float t)
{
    t = std::min(t, 1.0f);

    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec2 p = uuu * _start; //first term
    p += 3 * uu * t * _startOffset; //second term
    p += 3 * u * tt * _endOffset; //third term
    p += ttt * _end; //fourth term

    return p;
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

        _start = { position.x, position.z };
        _previous = _start;
        _startOffset = _start + glm::vec2{ forward.x, forward.z } * (float)positiveDistanceDist(randomEngine);

        _end = glm::vec2{ _start.x + (float)positionDist(randomEngine), _start.y + (float)positionDist(randomEngine) };
        _endOffset = _end + glm::vec2{ -forward.x * normal(randomEngine), -forward.z * normal(randomEngine) } * (float)positiveDistanceDist(randomEngine);

        _t = 0;
        _increase = ((float)newSpeed / glm::distance(_start, _end)) / 1000.0f;


        // TODO(gpascualg): Move this to somewhere else
        // HACK(gpascualg): Packet opcode is not known yet!
        Packet* broadcast = Packet::create(0x0A04);
        *broadcast << owner->id() << newSpeed;
        *broadcast << _start;
        
        Server::get()->map()->broadcastToSiblings(owner->cell(), broadcast);
        _hasPoint = true;

        // TODO(gpascualg): Move this to somewhere else
        // HACK(gpascualg): Packet opcode is not known yet!
        broadcast = Packet::create(0x0A05);
        *broadcast << owner->id();
        *broadcast << _start << _startOffset;
        *broadcast << _end << _endOffset;

        Server::get()->map()->broadcastToSiblings(owner->cell(), broadcast);

        // Start movement
        owner->motionMaster()->speed(newSpeed);
        owner->motionMaster()->move();

        LOG(LOG_MOVEMENT_GENERATOR, "Moving from (%.2f, %.2f) to (%.2f, %.2f)",
            _start.x, _start.y,
            _end.x, _end.y);

        LOG(LOG_MOVEMENT_GENERATOR, "Offsets from (%.2f, %.2f) to (%.2f, %.2f)",
            _startOffset.x, _startOffset.y,
            _endOffset.x, _endOffset.y);
    }

    _t += _increase * elapsed;
    auto nextPoint = next(_t);
    auto forward = glm::normalize(nextPoint - _previous);
    _previous = nextPoint;
    auto oldForward = owner->motionMaster()->forward();

    glm::vec3 forward3{ forward.x, oldForward.y, forward.y };
    owner->motionMaster()->forward(forward3);

    if (_t >= 1)
    {
        _hasPoint = false;

        LOG(LOG_MOVEMENT_GENERATOR, "Movement End");
    }

    return{ nextPoint.x, owner->motionMaster()->position().y, nextPoint.y };
}

bool RandomMovement::hasNext()
{
    return true;
}
