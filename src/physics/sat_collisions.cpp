/* Copyright 2016 Guillem Pascual */

#include "physics/sat_collisions.hpp"
#include "movement/motion_master.hpp"

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>


SAT* SAT::_instance = nullptr;

bool SAT::collides(BoundingBox* a, BoundingBox* b)
{
    if (a->Type == BoundingBoxType::RECT)
    {
        if (b->Type == BoundingBoxType::RECT)
        {
            return collides(*static_cast<RectBoundingBox*>(a), *static_cast<RectBoundingBox*>(b));
        }
        else if (b->Type == BoundingBoxType::CIRCULAR)
        {
            return collides(*static_cast<RectBoundingBox*>(a), *static_cast<CircularBoundingBox*>(b));
        }
    }
    else
    {
        if (b->Type == BoundingBoxType::RECT)
        {
            return collides(*static_cast<RectBoundingBox*>(b), *static_cast<CircularBoundingBox*>(a));
        }
        else if (b->Type == BoundingBoxType::CIRCULAR)
        {
            return collides(*static_cast<CircularBoundingBox*>(a), *static_cast<CircularBoundingBox*>(b));
        }
    }
}

bool SAT::collides(RectBoundingBox& a, RectBoundingBox& b)
{
    if (!collides(a.normals(), &a, &b))
    {
        return false;
    }

    return collides(b.normals(), &a, &b);
}

bool SAT::collides(RectBoundingBox& a, const CircularBoundingBox& b)
{
    // Find closest point from a to b
    float minDist = glm::length2(a._vertices[0] - b.center2D());
    glm::vec2 minVertex = a._vertices[0];

    for (int i = 1; i < a._vertices.size(); ++i)
    {
        float tmp = glm::length2(a._vertices[i] - b.center2D());
        if (tmp < minDist)
        {
            minDist = tmp;
            minVertex = a._vertices[i];
        }
    }

    // Collision based on circle normal?
    if (!collides({ minVertex - b.center2D() }, &a, &b))
    {
        return false;
    }

    // Collision based on rectangle edges
    return collides(a.normals(), &a, &b);
}

bool SAT::collides(const CircularBoundingBox& a, const CircularBoundingBox& b)
{
    return collides({ a.center2D() - b.center2D() }, &a, &b);
}

bool SAT::collides(const std::vector<glm::vec2>& axes, const BoundingBox* a, const BoundingBox* b)
{
    for (auto& axis : axes)
    {
        auto p1 = a->project(this, axis);
        auto p2 = b->project(this, axis);

        // TODO(gpascualg): It should be working, but test it!
        if (std::min(p1.y, p2.y) - std::max(p1.x, p2.x) < 0)
        {
            return false;
        }
    }

    return true;
}
