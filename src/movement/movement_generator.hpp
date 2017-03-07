/* Copyright 2016 Guillem Pascual */

#pragma once

#include <glm/glm.hpp>

class MapAwareEntity;

class MovementGenerator
{
public:
    virtual glm::vec3 update(MapAwareEntity* owner, float elapsed) = 0;
    virtual bool hasNext() = 0;
};


class RandomMovement : public MovementGenerator
{
public:
    RandomMovement();

    glm::vec2 next(float t);
    glm::vec3 update(MapAwareEntity* owner, float elapsed) override;
    bool hasNext() override;

private:
    bool _hasPoint;

    glm::vec2 _previous;
    glm::vec2 _start;
    glm::vec2 _end;
    glm::vec2 _startOffset;
    glm::vec2 _endOffset;

    float _t;
    float _increase;
};

