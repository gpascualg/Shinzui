/* Copyright 2016 Guillem Pascual */

#pragma once

#include "physics/bounding_box.hpp"
#include "physics/rect_bounding_box.hpp"
#include "physics/rotated_rect_bounding_box.hpp"
#include "physics/circular_bounding_box.hpp"

#include <initializer_list>
#include <vector>
#include <utility>

#include "defs/common.hpp"
#include "transform/fixed_transform.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
INCL_WARN


class CollisionsFramework
{
public:
    virtual bool collides(BoundingBox* a, glm::vec2& pos_a, BoundingBox* b, glm::vec2& pos_b) = 0;

    inline bool collides(FixedTransform& a, FixedTransform& b);

    glm::vec2 project(const RotatedRectBoundingBox& bb, glm::vec2 axis, glm::vec2 pos);
    glm::vec2 project(const RectBoundingBox& bb, glm::vec2 axis, glm::vec2 pos);
    glm::vec2 project(const CircularBoundingBox& bb, glm::vec2 axis, glm::vec2 pos);
};


bool CollisionsFramework::collides(FixedTransform& a, FixedTransform& b)
{
    return collides(a.BBox, a.Position2D, b.BBox, b.Position2D);
}
