/* Copyright 2016 Guillem Pascual */

#pragma once

#include <initializer_list>
#include <vector>
#include <utility>
#include <glm/glm.hpp>


class MotionMaster;

class BoundingBox
{
public:
	BoundingBox(MotionMaster* motionMaster);

    // Rotate with motion master
	virtual void rotate(float angle) = 0;

    // Returns the minimum rect that contains the OBB
    virtual glm::vec4 asRect() = 0;

    // Intersection with a segment
    virtual bool intersects(glm::vec2 p0, glm::vec2 p1, float* dist) = 0;

protected:
    // Normals of the edges (if any)
    virtual const std::vector<glm::vec2>& normals() = 0;

protected:
    MotionMaster* _motionMaster;
};
