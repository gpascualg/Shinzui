/* Copyright 2016 Guillem Pascual */

#pragma once

#include <initializer_list>
#include <vector>
#include <utility>

#include "defs/common.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
INCL_WARN


class MotionMaster;
class CollisionsFramework;

enum class BoundingBoxType
{
    RECT,
    CIRCULAR
};

class BoundingBox
{
public:
    BoundingBox(BoundingBoxType type);

    // Rotate with motion master
    virtual void rotate(float angle) = 0;

    // Returns the minimum rect that contains the OBB
    virtual glm::vec4 rect(glm::vec2 pos) = 0;

    // Intersection with a segment
    virtual bool intersects(glm::vec2 pos, glm::vec2 p0, glm::vec2 p1, float* dist) = 0;

    // Collisions
    virtual glm::vec2 project(CollisionsFramework* framework, glm::vec2 axis, glm::vec2 pos) const = 0;

protected:
    // Normals of the edges (if any)
    virtual const std::vector<glm::vec2>& normals() = 0;

public:
    const BoundingBoxType Type;
};
