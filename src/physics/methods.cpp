/* Copyright 2016 Guillem Pascual */

#include "physics/methods.hpp"


// TODO(gpascualg): Benchmark different segment-segment intersection algos
bool ccw(glm::vec2 A, glm::vec2 B, glm::vec2 C)
{
    return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
}

bool intersects(glm::vec2 A, glm::vec2 B, glm::vec2 C, glm::vec2 D)
{
    return ccw(A, C, D) != ccw(B, C, D) && ccw(A, B, C) != ccw(A, B, D);
}
