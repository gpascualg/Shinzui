/* Copyright 2016 Guillem Pascual */

#pragma once

#include "defs/common.hpp"
#include "physics/bounding_box.hpp"
#include "transform/transform.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
INCL_WARN


struct FixedTransform
{
public:
    FixedTransform(TimePoint t, Transform& transform, BoundingBox* bbox);
    FixedTransform(const FixedTransform&) = delete;
    ~FixedTransform();

    glm::vec4 rect();

    TimePoint TimeRef;
    BoundingBox* BBox;
    glm::vec3 Position;
    glm::vec2 Position2D;
    glm::vec3 Forward;
    float Speed;
};
