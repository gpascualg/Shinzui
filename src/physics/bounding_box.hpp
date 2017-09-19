/* Copyright 2016 Guillem Pascual */

#pragma once

#include <initializer_list>
#include <list>
#include <utility>
#include <glm/glm.hpp>


class BoundingBox
{
public:
	BoundingBox();
	BoundingBox(std::initializer_list<glm::vec3>&& vertices);
	void rotate(float angle);

	inline void setVertices(std::initializer_list<glm::vec3>&& vertices);

private:
	std::list<glm::vec3> _vertices;
};

void BoundingBox::setVertices(std::initializer_list<glm::vec3>&& vertices)
{
	_vertices = std::move(vertices);
}
