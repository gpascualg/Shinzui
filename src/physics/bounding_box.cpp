/* Copyright 2016 Guillem Pascual */

#include "physics/bounding_box.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>


BoundingBox::BoundingBox()
{}

BoundingBox::BoundingBox(std::initializer_list<glm::vec3>&& vertices):
	BoundingBox{}
{
	setVertices(std::move(vertices));
}

void BoundingBox::rotate(float angle)
{
	for (auto& vertix : _vertices)
	{
		vertix = glm::normalize(glm::rotateY(vertix, angle));
	}
}

