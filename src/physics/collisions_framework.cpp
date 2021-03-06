/* Copyright 2016 Guillem Pascual */

#include "movement/motion_master.hpp"
#include "physics/collisions_framework.hpp"

#include "defs/common.hpp"

INCL_NOWARN
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
INCL_WARN


glm::vec2 CollisionsFramework::project(const RectBoundingBox& bb, glm::vec2 axis)
{
    float min = glm::dot(axis, bb._vertices[0] + bb.position2D());
    float max = min;
    for (uint32_t i = 1; i < bb._vertices.size(); ++i)
    {
        float tmp = glm::dot(axis, bb._vertices[i] + bb.position2D());
        if (tmp < min)
        {
            min = tmp;
        }
        else if (tmp > max)
        {
            max = tmp;
        }
    }

    return { min, max };
}

glm::vec2 CollisionsFramework::project(const CircularBoundingBox& bb, glm::vec2 axis)
{
    // Project center onto line
    auto p0 = bb.center2D() + bb.position2D() - axis * bb.radius();
    auto p1 = bb.center2D() + bb.position2D() + axis * bb.radius();

    auto r0 = glm::dot(axis, p0);
    auto r1 = glm::dot(axis, p1);

    if (r0 < r1)
    {
        return { r0, r1 };
    }

    return { r1, r0 };
}
