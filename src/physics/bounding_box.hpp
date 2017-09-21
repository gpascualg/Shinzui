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
    bool overlaps(BoundingBox* other);

private:
    glm::vec2 project(glm::vec2 axis);

private:
    MotionMaster* _motionMaster;
    bool _recalcNormals;
	std::vector<glm::vec2> _vertices;
    std::vector<glm::vec2> _normals;
};

void BoundingBox::setVertices(std::initializer_list<glm::vec2>&& vertices)
{
	_vertices = std::move(vertices);
}
