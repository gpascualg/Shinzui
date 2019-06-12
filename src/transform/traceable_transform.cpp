#include "transform/traceable_transform.hpp"
#include "physics/bounding_box.hpp"


// Consider at most 50 updates per tick (2500 ms)
// Cache up to 25 queries
TraceableTransform::TraceableTransform() :
    buffer(50),
    cache(25)
{}

FixedTransform& TraceableTransform::at(TimePoint t, BoundingBox* bbox)
{
    for (auto& existing : cache)
    {
        if (existing.TimeRef == t)
        {
            return existing;
        }
    }

    cache.emplace_back(t, *iterator(t), bbox);
    return cache.back();
}

void TraceableTransform::clear()
{
    buffer.clear();
}

void TraceableTransform::teleport(TimePoint t, glm::vec3& position, glm::vec3& forward)
{
    buffer.emplace_back(t, position, forward, 0);
}

void TraceableTransform::move(TimePoint t, float speed)
{
    FixedTransform& transform = at(t, nullptr);
    buffer.emplace_back(t, transform.Position, transform.Forward, speed);
}

void TraceableTransform::rotate(TimePoint t, float angle)
{
    FixedTransform& transform = at(t, nullptr);
    buffer.emplace_back(t, transform.Position, transform.Forward, transform.Speed);
    buffer.back().startRotation(angle);
}

void TraceableTransform::stop(TimePoint t)
{
    move(t, 0);
}

boost::circular_buffer<Transform>::iterator TraceableTransform::iterator(TimePoint t)
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
