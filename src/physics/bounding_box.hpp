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
	BoundingBox(MotionMaster* motionMaster, std::initializer_list<glm::vec2>&& vertices);
	void rotate(float angle);

	inline void setVertices(std::initializer_list<glm::vec2>&& vertices);
    std::vector<glm::vec2>& normals();

    glm::vec4 asRect();

    bool overlaps(BoundingBox* other);
    bool intersects(glm::vec2 s1_s, glm::vec2 s1_e);

private:
    glm::vec2 project(glm::vec2 axis);

private:
    MotionMaster* _motionMaster;
    bool _recalcNormals;
	std::vector<glm::vec2> _vertices;
    std::vector<glm::vec2> _normals;

    glm::vec2 _min;
    glm::vec2 _max;
};

void BoundingBox::setVertices(std::initializer_list<glm::vec2>&& vertices)
{
	_vertices = std::move(vertices);
}
