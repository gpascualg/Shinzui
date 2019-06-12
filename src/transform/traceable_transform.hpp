/* Copyright 2016 Guillem Pascual */

#pragma once

#include "defs/common.hpp"
#include "transform/fixed_transform.hpp"
#include "transform/transform.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
#include <boost/circular_buffer.hpp>
INCL_WARN


class BoundingBox;


class TraceableTransform
{
public:
    TraceableTransform();

    FixedTransform& at(TimePoint t, BoundingBox* bbox);
    void clear();

    void teleport(TimePoint t, glm::vec3& position, glm::vec3& forward);
    void move(TimePoint t, float speed);
    void rotate(TimePoint t, float speed);
    void stop(TimePoint t);

protected:
    boost::circular_buffer<Transform>::iterator iterator(TimePoint t);

private:
    boost::circular_buffer<Transform> buffer;
    boost::circular_buffer<FixedTransform> cache;
};
