#include <cstdlib>
#include <cmath>

#include "defs/common.hpp"
#include "physics/bounding_box.hpp"

INCL_NOWARN
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include <boost/circular_buffer.hpp>
INCL_WARN


class Transform;
struct FixedTransform;
class TraceableTransform;

struct FixedTransform
{
public:
    FixedTransform(TimePoint t, Transform& transform);

    glm::vec3 Position;
    glm::vec2 Position2D;
    glm::vec3 Forward;
    float Speed;
};

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
    Transform(TimePoint t):
        _t0(t)
    {}

    inline auto t() { return _t0; }
    inline bool isMoving() { return (_flags & (uint8_t)MovementFlags::MOVING) == (uint8_t)MovementFlags::MOVING; }
    inline bool isRotating() { return (_flags & (uint8_t)MovementFlags::ROTATING) == (uint8_t)MovementFlags::ROTATING; }

    float speed(TimePoint t)
    {
        // TODO(gpascualg): Acceleration?
        return _speed;
    }

    inline glm::vec2 position2D(TimePoint t)
    {
        glm::vec3 pos = position(t);
        return { pos.x, pos.z };
    }

    glm::vec3 position(TimePoint t1)
    {
        // If it has stoped, simply return the same point
        if (!isMoving())
        {
            return _position;
        }

        auto time = elapsed(t1);

        if (isRotating())
        {
            auto angle = _initialAngle + _angle * time;
            return _center + glm::vec3 { _radius * std::cos(angle), 0, _radius * std::sin(angle) };
        }

        return _position + _forward * _speed * time;
    }

    glm::vec3 forward(TimePoint t1)
    {
        if (!isRotating())
        {
            return _forward;
        }
        
        return glm::normalize(glm::rotateY(_forward, _angle * elapsed(t1)));
    }

    void startRotation(float angle)
    {
        _flags = _flags | (uint8_t)MovementFlags::ROTATING;
        _angle = angle;
        _initialAngle = std::atan2(_position.z, _position.x);
        _center = glm::vec3(_position.x - std::cos(_initialAngle), 0, _position.z - std::sin(_initialAngle));
        recalcParameters();
    }

    void setSpeed(float speed)
    {
        _speed = speed;
        recalcParameters();
    }

private:
    void recalcParameters()
    {
        if (isRotating())
        {
            _radius = _speed / _angle;
        }
    }

    inline float elapsed(TimePoint t1)
    {
        return static_cast<float>((t1 - _t0).count());
    }

private:
    glm::vec3 _position;
    glm::vec3 _forward;
    glm::vec3 _center;
    uint8_t _flags;
    float _speed;
    float _initialAngle;
    float _angle;
    float _radius;
    TimePoint _t0;
};


class TraceableTransform
{
public:
    // Consider at most 50 updates per tick
    // which is already a lot!
    TraceableTransform() :
        buffer(50)
    {}

    FixedTransform at(TimePoint t)
    {
        return FixedTransform(t, *iterator(t));
    }

    void clear()
    {
        buffer.clear();
    }

protected:
    boost::circular_buffer<Transform>::iterator iterator(TimePoint t)
    {
        // Maybe last is already fine?
        if (buffer.back().t() <= t)
        {
            return --(buffer.end());
        }

        // Otherwise, search for it
        auto it = buffer.begin();
        while (it != buffer.end())
        {
            if ((*it).t() >= t)
            {
                break;
            }

            ++it;
        }

        return it;
    }

private:
    boost::circular_buffer<Transform> buffer;
};
