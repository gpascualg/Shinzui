/* Copyright 2016 Guillem Pascual */

#include "debug/debug.hpp"
#include "physics/methods.hpp"
#include "physics/collisions_framework.hpp"
#include "physics/rect_bounding_box.hpp"
#include "movement/motion_master.hpp"

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>


RectBoundingBox::RectBoundingBox(MotionMaster* motionMaster, std::initializer_list<glm::vec2>&& vertices) :
    BoundingBox{ motionMaster, BoundingBoxType::RECT },
    _recalcNormals(false),
    _vertices(std::move(vertices))
{
    _normals.resize(2);
}

void RectBoundingBox::rotate(float angle)
{
	for (auto& vertix : _vertices)
	{
        // TODO(gpascualg): Is rotate 2D equivalent to rotateY?
		vertix = glm::rotate(vertix, angle);
	}

    _recalcNormals = true;
}

const std::vector<glm::vec2>& RectBoundingBox::normals()
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

glm::vec4 RectBoundingBox::asRect()
{
    const auto pos = motionMaster()->position2D();
    normals(); // Force recalc

    return { _min.x + pos.x, _min.y + pos.y, _max.x + pos.x, _max.y + pos.y };
}

bool RectBoundingBox::intersects(glm::vec2 s1_s, glm::vec2 s1_e, float* dist)
{
    bool check = false;

    if (dist)
    {
        *dist = 0;
    }

    for (int i = 0; i < _vertices.size(); ++i)
    {
        auto s0_s = _vertices[i] + motionMaster()->position2D();
        auto s0_e = _vertices[(i + 1) % _vertices.size()] + motionMaster()->position2D();

        /*LOG(LOG_FIRE_LOGIC, "Intersection with (%f,%f)\n\t(%f,%f)-(%f,%f) to (%f,%f)-(%f,%f)", _motionMaster->position2D().x, _motionMaster->position2D().y,
        s0_s.x, s0_s.y, s0_e.x, s0_e.y, s1_s.x, s1_s.y, s1_e.x, s1_e.y);*/

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

glm::vec2 RectBoundingBox::project(CollisionsFramework* framework, glm::vec2 axis) const
{
    return framework->project(*this, axis);
}

