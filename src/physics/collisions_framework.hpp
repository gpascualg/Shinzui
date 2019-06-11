/* Copyright 2016 Guillem Pascual */

#pragma once

#include "physics/bounding_box.hpp"
#include "physics/rect_bounding_box.hpp"
#include "physics/circular_bounding_box.hpp"

#include <initializer_list>
#include <vector>
#include <utility>

#include "defs/common.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
INCL_WARN


class CollisionsFramework
{
public:
    virtual bool collides(BoundingBox* a, glm::vec2 pos_a, BoundingBox* b, glm::vec2 pos_b) = 0;

    glm::vec2 project(const RectBoundingBox& bb, glm::vec2 axis, glm::vec2 pos);
    glm::vec2 project(const CircularBoundingBox& bb, glm::vec2 axis, glm::vec2 pos);
};
