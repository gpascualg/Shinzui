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
    BoundingBox(MotionMaster* motionMaster, BoundingBoxType type);
    BoundingBox(const glm::vec3& position, BoundingBoxType type);

    // Rotate with motion master
    virtual void rotate(float angle) = 0;

    // Returns the minimum rect that contains the OBB
    virtual glm::vec4 asRect() = 0;

    // Intersection with a segment
    virtual bool intersects(glm::vec2 p0, glm::vec2 p1, float* dist) = 0;

    // Collisions
    virtual glm::vec2 project(CollisionsFramework* framework, glm::vec2 axis) const = 0;

    inline const glm::vec3& position() const { return _position; }
    inline const glm::vec2 position2D() const { return { _position.x, _position.z }; }

protected:
    // Normals of the edges (if any)
    virtual const std::vector<glm::vec2>& normals() = 0;

public:
    const BoundingBoxType Type;

private:
    const glm::vec3& _position;
};
