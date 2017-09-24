/* Copyright 2016 Guillem Pascual */

#include "debug/debug.hpp"
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
        _min = _vertices[0];
        _max = _vertices[0];

        for (int i = 0; i < 4; ++i)
        {
            if (i < 2)
            {
                auto tmp = _vertices[i] - _vertices[i + 1];
                _normals[i] = glm::vec2(-tmp.y, tmp.x); // (-y, x) || (y, -x)
            }

            if (i > 0)
            {
                if (_vertices[i].x < _min.x) _min.x = _vertices[i].x;
                else if (_vertices[i].x > _max.x) _max.x = _vertices[i].x;

                if (_vertices[i].y < _min.y) _min.y = _vertices[i].y;
                else if (_vertices[i].y > _max.y) _max.y = _vertices[i].y;
            }
        }

        _recalcNormals = false;
    }

    return _normals;
}

glm::vec4 BoundingBox::asRect()
{
    const auto pos = _motionMaster->position2D();
    normals(); // Force recalc

    return { _min.x + pos.x, _min.y + pos.y, _max.x + pos.x, _max.y + pos.y };
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

// TODO(gpascualg): Benchmark different segment-segment intersection algos
bool ccw(glm::vec2 A, glm::vec2 B, glm::vec2 C)
{
    return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
}

bool intersects(glm::vec2 A, glm::vec2 B, glm::vec2 C, glm::vec2 D)
{
    return ccw(A, C, D) != ccw(B, C, D) && ccw(A, B, C) != ccw(A, B, D);
}

bool BoundingBox::intersects(glm::vec2 s1_s, glm::vec2 s1_e, float* dist)
{
    bool check = false;

    if (dist)
    {
        *dist = 0;
    }

    for (int i = 0; i < _vertices.size(); ++i)
    {
        auto& s0_s = _vertices[i] + _motionMaster->position2D();
        auto& s0_e = _vertices[(i + 1) % _vertices.size()] + _motionMaster->position2D();

        if (::intersects(s0_s, s0_e, s1_s, s1_e))
        {
            // If dist is not needed, return right away
            if (!dist)
            {
                return true;
            }
            else
            {
                // Calculate dist
                float d = std::sqrt(std::pow(s0_e.y - s0_s.y, 2) + std::pow(s0_e.x - s0_s.x, 2));
                if (std::abs(d) <= glm::epsilon<float>())
                {
                    float tmp = ((s0_e.y - s0_s.y) * s1_s.x - (s0_e.x - s0_s.x) * s1_s.y + s0_e.x * s0_s.y - s0_e.y * s0_s.x) / d;
                    if (!check || tmp < *dist)
                    {
                        *dist = tmp;
                    }
                }
                else
                {
                    *dist = 0;
                }
            }

            check = true;
        }
    }

    return check;
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
