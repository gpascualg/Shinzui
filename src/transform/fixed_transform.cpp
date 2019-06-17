#include <cstdlib>
#include <cmath>
#include <inttypes.h>

#include "transform/fixed_transform.hpp"
#include "debug/debug.hpp"


FixedTransform::FixedTransform(TimePoint t, Transform& transform, BoundingBox* bbox) :
    TimeRef(t),
    BBox(nullptr),
    Position(transform.position(t)),
    Position2D(transform.position2D(t)),
    Forward(transform.forward(t)),
    Speed(transform.speed(t)),
    IsMoving(transform.isMoving()),
    IsRotating(transform.isRotating())
{
    if (bbox)
    {
        BBox = bbox->rotate(transform.rotation(t));
    }
}

FixedTransform::~FixedTransform()
{
    // Circular bounding boxes don't return new instances on rotations
    if (BBox && BBox->Type != BoundingBoxType::CIRCULAR)
    {
        delete BBox;
    }
}

glm::vec4 FixedTransform::rect()
{
    LOG_ASSERT(BBox != nullptr, "Trying to use a transform without BBox");
    return BBox->rect(Position2D);
}
