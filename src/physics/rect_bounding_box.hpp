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

class RectBoundingBox : public BoundingBox
{
    friend class CollisionsFramework;
    friend class SAT;

public:
    RectBoundingBox(std::initializer_list<glm::vec2>&& vertices);

    void rotate(float angle) override;
    glm::vec4 rect(glm::vec2 pos) override;
    bool intersects(glm::vec2 pos, glm::vec2 s1_s, glm::vec2 s1_e, float* dist = nullptr) override;
    glm::vec2 project(CollisionsFramework* framework, glm::vec2 axis, glm::vec2 pos) const override;

protected:
    const std::vector<glm::vec2>& normals() override;

private:
    bool _recalcNormals;
    std::vector<glm::vec2> _vertices;
    std::vector<glm::vec2> _normals;

    glm::vec2 _min;
    glm::vec2 _max;
};
