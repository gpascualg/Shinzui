#include "time/traceable.hpp"

FixedTransform::FixedTransform(TimePoint t, Transform& transform):
    Position(transform.position(t)),
    Position2D(transform.position2D(t)),
    Forward(transform.forward(t)),
    Speed(transform.speed(t))
{}
