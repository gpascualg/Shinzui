/* Copyright 2016 Guillem Pascual */

#include "debug/debug.hpp"
#include "physics/circular_bounding_box.hpp"
#include "physics/collisions_framework.hpp"
#include "movement/motion_master.hpp"

#include <algorithm>

#include "defs/common.hpp"

INCL_NOWARN
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
INCL_WARN


CircularBoundingBox::CircularBoundingBox(MotionMaster* motionMaster, glm::vec3 center, float radius) :
    BoundingBox{ motionMaster, BoundingBoxType::CIRCULAR },
    _center(center),
    _radius(radius)
{}

CircularBoundingBox::CircularBoundingBox(const glm::vec3& position, glm::vec3 center, float radius) :
    BoundingBox{ position, BoundingBoxType::CIRCULAR },
    _center(center),
    _radius(radius)
{}

void CircularBoundingBox::rotate(float angle)
{
	// No need to do anything! Rotating does not change the center
}

const std::vector<glm::vec2>& CircularBoundingBox::normals()
{
    // TODO(gpascualg): What should we do here? A circle has no normals... :(
    assert(false);
    return {};
}

glm::vec4 CircularBoundingBox::asRect()
{
    const auto pos = position2D();

    return {
        _center.x + pos.x - _radius,
        _center.y + pos.y - _radius,
        _center.x + pos.x + _radius,
        _center.y + pos.y + _radius
    };
}

// http://csharphelper.com/blog/2014/09/determine-where-a-line-intersects-a-circle-in-c/
bool CircularBoundingBox::intersects(glm::vec2 p0, glm::vec2 p1, float* dist)
{
    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;

    float A = std::pow(dx, 2) + std::pow(dy, 2);

    // This basically checks whether p0==p1
    if (std::abs(A) <= glm::epsilon<float>())
    {
        return false;
    }

    float cx = position2D().x + _center.x;
    float cy = position2D().y + _center.y;

    float B = 2 * (dx * (p0.x - cx) + dy * (p0.y - cy));
    float C = std::pow(p0.x - cx, 2) + std::pow(p0.y - cy, 2) - std::pow(_radius, 2);
    float det = std::pow(B, 2) - 4 * A * C;

    if (det < 0)
    {
        return false;
    }

    return true;
}

glm::vec2 CircularBoundingBox::project(CollisionsFramework* framework, glm::vec2 axis) const
{
    return framework->project(*this, axis);
}
