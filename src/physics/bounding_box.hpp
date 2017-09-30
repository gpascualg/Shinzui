/* Copyright 2016 Guillem Pascual */

#pragma once

#include <initializer_list>
#include <vector>
#include <utility>
#include <glm/glm.hpp>


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
    BoundingBox(const glm::vec2& position, BoundingBoxType type);

    // Rotate with motion master
	virtual void rotate(float angle) = 0;

    // Returns the minimum rect that contains the OBB
    virtual glm::vec4 asRect() = 0;

    // Intersection with a segment
    virtual bool intersects(glm::vec2 p0, glm::vec2 p1, float* dist) = 0;

    // Collisions
    virtual glm::vec2 project(CollisionsFramework* framework, glm::vec2 axis) const = 0;
    
    //inline MotionMaster* motionMaster() const { return _motionMaster; }
    inline const glm::vec2& position() const { return _position; }

protected:
    // Normals of the edges (if any)
    virtual const std::vector<glm::vec2>& normals() = 0;

public:
    const BoundingBoxType Type;

private:
    MotionMaster* _motionMaster;
    const glm::vec2& _position;
};
