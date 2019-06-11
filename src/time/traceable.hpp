#include <cstdlib>

#include "defs/common.hpp"
#include "physics/bounding_box.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
#include <boost/circular_buffer.hpp>
INCL_WARN


template <typename T>
class Traceable
{
public:
    // Consider at most 50 updates per tick
    // which is already a lot!
    Traceable() :
        buffer(50)
    {}

    T& at(TimePoint t)
    {
        return *iterator(t);
    }

    template <typename... Args>
    T& set(TimePoint t, Args... args)
    {
        T o = T(t, args...);
        buffer.push_back(o);
        return o;
    }

    template <typename... Args>
    T& exactly(TimePoint t, Args... args)
    {
        if (buffer.empty())
        {
            return set(t, args...);
        }

        T& at = *iterator(t);
        if (at.t0 == t)
        {
            return at;
        }

        return set(t, args...);
    }

    void clear()
    {
        buffer.clear();
    }

protected:
    auto iterator(TimePoint t)
    {
        // Maybe last is already fine?
        if (buffer.back().t0 <= t)
        {
            return --(buffer.end());
        }

        // Otherwise, search for it
        auto it = buffer.begin();
        while (it != buffer.end())
        {
            if ((*it).t0 >= t)
            {
                break;
            }

            ++it;
        }

        return it;
    }

private:
    boost::circular_buffer<T> buffer;
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
        t0(t)
    {}

    glm::vec3 position;
    glm::vec3 forward;
    uint8_t flags;
    float speed;
    TimePoint t0;

    inline bool isMoving() { return (flags & (uint8_t)MovementFlags::MOVING) == (uint8_t)MovementFlags::MOVING; }
    inline bool isRotating() { return (flags & (uint8_t)MovementFlags::ROTATING) == (uint8_t)MovementFlags::ROTATING; }

    inline glm::vec2 to2D(TimePoint t)
    {
        glm::vec3 position = get(t);
        return { position.x, position.z };
    }

    glm::vec3 get(TimePoint t1)
    {
        // If it has stoped, simply return the same point
        if (std::abs(speed) < 1e-6)
        {
            return position;
        }

        return position + forward * speed * static_cast<float>((t1 - t0).count());
    }
};

template <typename T>
struct Fixed
{
    template <typename... Args>
    Fixed(TimePoint t, Args... args):
        t0(t),
        value(args...)
    {}

    TimePoint t0;
    T value;
    T get(TimePoint t1) { return value; }
};
