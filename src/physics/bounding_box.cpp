/* Copyright 2016 Guillem Pascual */

#include "physics/bounding_box.hpp"
#include "movement/motion_master.hpp"

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>


BoundingBox::BoundingBox(MotionMaster* motionMaster) :
    _motionMaster(motionMaster),
    _recalcNormals(false)
{
    _normals.resize(2);
}

BoundingBox::BoundingBox(MotionMaster* motionMaster, std::initializer_list<glm::vec2>&& vertices) :
	BoundingBox{motionMaster}
{
	setVertices(std::move(vertices));
}

void BoundingBox::rotate(float angle)
{
	for (auto& vertix : _vertices)
	{
        // TODO(gpascualg): Is rotate 2D equivalent to rotateY?
		vertix = glm::normalize(glm::rotate(vertix, angle));
	}

    _recalcNormals = true;
}

std::vector<glm::vec2>& BoundingBox::normals()
{
    if (_recalcNormals)
    {
        for (int i = 0; i < 2; ++i)
        {
            auto tmp = _vertices[i] - _vertices[i + 1];
            _normals[i] = glm::vec2(-tmp.y, tmp.x); // (-y, x) || (y, -x)
        }

        _recalcNormals = false;
    }

    return _normals;
}

bool BoundingBox::overlaps(BoundingBox* other)
{
    auto& axes1 = normals();
    for (auto& axis : axes1)
    {
        auto p1 = project(axis);
        auto p2 = other->project(axis);

        if (std::min(p1.y, p2.y) - std::max(p1.x, p2.x) < 0)
        {
            return false;
        }
    }

    auto& axes2 = other->normals();
    for (auto& axis : axes2)
    {
        auto p1 = project(axis);
        auto p2 = other->project(axis);

        if (std::min(p1.y, p2.y) - std::max(p1.x, p2.x) < 0)
        {
            return false;
        }
    }

    return true;
}

glm::vec2 BoundingBox::project(glm::vec2 axis)
{
    float min = glm::dot(axis, _vertices[0] + _motionMaster->position2D());
    float max = min;
    for (int i = 1; i < _vertices.size(); ++i)
    {
        float tmp = glm::dot(axis, _vertices[i] + _motionMaster->position2D());
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
