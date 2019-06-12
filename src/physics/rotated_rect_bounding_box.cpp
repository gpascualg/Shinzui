/* Copyright 2016 Guillem Pascual */

#include "debug/debug.hpp"
#include "physics/methods.hpp"
#include "physics/collisions_framework.hpp"
#include "physics/rect_bounding_box.hpp"
#include "physics/rotated_rect_bounding_box.hpp"
#include "movement/motion_master.hpp"

#include <algorithm>
#include <utility>
#include <vector>

#include "defs/common.hpp"

INCL_NOWARN
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
INCL_WARN


RotatedRectBoundingBox::RotatedRectBoundingBox(RectBoundingBox* bbox, std::vector<glm::vec2> vertices, float angle) :
    BoundingBox{ BoundingBoxType::RECT },
    _bbox(bbox),
    _recalcNormals(true),
    _vertices(vertices)
{
    // Rotate vertices
    for (auto& vertix : _vertices)
    {
        // TODO(gpascualg): Do not immediatelly rotate, accumulate and do so in demand
        vertix = glm::rotate(vertix, angle);
    }

    // Resize normals and calculate them
    _normals.resize(2);
    normals();
}

BoundingBox* RotatedRectBoundingBox::rotate(float angle)
{
    LOG_ASSERT(false, "RotatedRectBoundingBox can not be rotated");
    return this;
}

const std::vector<glm::vec2>& RotatedRectBoundingBox::normals()
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
                _normals[i] = glm::vec2(-tmp.y, tmp.x);  // (-y, x) || (y, -x)
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

glm::vec4 RotatedRectBoundingBox::rect(glm::vec2 pos)
{
    // Normals are already calculated
    return { _min.x + pos.x, _min.y + pos.y, _max.x + pos.x, _max.y + pos.y };
}

bool RotatedRectBoundingBox::intersects(glm::vec2 pos, glm::vec2 s1_s, glm::vec2 s1_e, float* dist)
{
    return _bbox->intersects(_vertices, pos, s1_s, s1_e, dist);
}

glm::vec2 RotatedRectBoundingBox::project(CollisionsFramework* framework, glm::vec2 axis, glm::vec2 pos) const
{
    return framework->project(*this, axis, pos);
}
