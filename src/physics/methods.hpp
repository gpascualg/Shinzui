/* Copyright 2016 Guillem Pascual */

#pragma once

#include "physics/bounding_box.hpp"

#include <initializer_list>
#include <vector>
#include <utility>
#include <glm/glm.hpp>


bool intersects(glm::vec2 A, glm::vec2 B, glm::vec2 C, glm::vec2 D);
