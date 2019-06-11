/* Copyright 2016 Guillem Pascual */

#pragma once

#include "physics/bounding_box.hpp"

#include <initializer_list>
#include <vector>
#include <utility>

#include "defs/common.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
INCL_WARN


class MotionMaster;

class CircularBoundingBox : public BoundingBox
{
    friend class CollisionsFramework;
    friend class SAT;

public:
    CircularBoundingBox(glm::vec3 center, float radius);

    void rotate(float angle) override;
    glm::vec4 rect(glm::vec2 pos) override;
    bool intersects(glm::vec2 pos, glm::vec2 p0, glm::vec2 p1, float* dist = nullptr) override;
    glm::vec2 project(CollisionsFramework* framework, glm::vec2 axis, glm::vec2 pos) const override;

    const inline glm::vec3& center() const { return _center; }
    const inline glm::vec2 center2D() const { return { _center.x, _center.z }; }
    inline float radius() const { return _radius; }

protected:
    const std::vector<glm::vec2>& normals() override;

private:
    glm::vec3 _center;
    float _radius;
};
