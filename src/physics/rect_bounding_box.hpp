/* Copyright 2016 Guillem Pascual */

#pragma once

#include "physics/bounding_box.hpp"

#include <initializer_list>
#include <vector>
#include <utility>
#include <glm/glm.hpp>


class MotionMaster;

class RectBoundingBox : public BoundingBox
{
public:
    RectBoundingBox(MotionMaster* motionMaster);
    inline void setVertices(std::initializer_list<glm::vec2>&& vertices);

	void rotate(float angle) override;
    glm::vec4 asRect() override;
    bool intersects(glm::vec2 s1_s, glm::vec2 s1_e, float* dist = nullptr) override;

protected:
    const std::vector<glm::vec2>& normals() override;

private:
    bool _recalcNormals;
	std::vector<glm::vec2> _vertices;
    std::vector<glm::vec2> _normals;

    glm::vec2 _min;
    glm::vec2 _max;
};

void RectBoundingBox::setVertices(std::initializer_list<glm::vec2>&& vertices)
{
	_vertices = std::move(vertices);
}
