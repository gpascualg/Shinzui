/* Copyright 2016 Guillem Pascual */

#include "physics/sat_collisions.hpp"
#include "movement/motion_master.hpp"

#include <algorithm>
#include <vector>

#include "defs/common.hpp"

INCL_NOWARN
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>
INCL_WARN


SAT* SAT::_instance = nullptr;

bool SAT::collides(BoundingBox* a, glm::vec2& pos_a, BoundingBox* b, glm::vec2& pos_b)
{
    if (a->Type == BoundingBoxType::RECT)
    {
        if (b->Type == BoundingBoxType::RECT)
        {
            return collides(*static_cast<RectBoundingBox*>(a), pos_a, *static_cast<RectBoundingBox*>(b), pos_b);
        }
        else if (b->Type == BoundingBoxType::CIRCULAR)
        {
            return collides(*static_cast<RectBoundingBox*>(a), pos_a, *static_cast<CircularBoundingBox*>(b), pos_b);
        }
    }
    else
    {
        if (b->Type == BoundingBoxType::RECT)
        {
            return collides(*static_cast<RectBoundingBox*>(b), pos_a, *static_cast<CircularBoundingBox*>(a), pos_b);
        }
        else if (b->Type == BoundingBoxType::CIRCULAR)
        {
            return collides(*static_cast<CircularBoundingBox*>(a), pos_a, *static_cast<CircularBoundingBox*>(b), pos_b);
        }
    }

    return false;
}

bool SAT::collides(RectBoundingBox& a, glm::vec2& pos_a, RectBoundingBox& b, glm::vec2& pos_b)
{
    if (!collides(a.normals(), &a, pos_a, &b, pos_b))
    {
        return false;
    }

    return collides(b.normals(), &a, pos_a, &b, pos_b);
}

bool SAT::collides(RectBoundingBox& a, glm::vec2& pos_a, const CircularBoundingBox& b, glm::vec2& pos_b)
{
    // Find closest point from a to b
    float minDist = glm::length2(a._vertices[0] - b.center2D());
    glm::vec2& minVertex = a._vertices[0];

    for (uint32_t i = 1; i < a._vertices.size(); ++i)
    {
        float tmp = glm::length2(a._vertices[i] - b.center2D());
        if (tmp < minDist)
        {
            minDist = tmp;
            minVertex = a._vertices[i];
        }
    }

    // Collision based on circle normal?
    if (!collides({ minVertex - b.center2D() }, &a, pos_a, &b, pos_b))  // NOLINT (whitespace/braces)
    {
        return false;
    }

    // Collision based on rectangle edges
    return collides(a.normals(), &a, pos_a, &b, pos_b);
}

bool SAT::collides(const CircularBoundingBox& a, glm::vec2& pos_a, const CircularBoundingBox& b, glm::vec2& pos_b)
{
    return collides({ a.center2D() - b.center2D() }, &a, pos_a, &b, pos_b);  // NOLINT (whitespace/braces)
}

bool SAT::collides(const std::vector<glm::vec2>& axes, const BoundingBox* a, glm::vec2& pos_a, const BoundingBox* b, glm::vec2& pos_b)
{
    for (auto& axis : axes)
    {
        auto p1 = a->project(this, axis, pos_a);
        auto p2 = b->project(this, axis, pos_b);

        // TODO(gpascualg): It should be working, but test it!
        if (std::min(p1.y, p2.y) - std::max(p1.x, p2.x) < 0)
        {
            return false;
        }
    }

    return true;
}
