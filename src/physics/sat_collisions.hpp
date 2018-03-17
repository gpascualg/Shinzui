/* Copyright 2016 Guillem Pascual */

#pragma once

#include "physics/bounding_box.hpp"
#include "physics/circular_bounding_box.hpp"
#include "physics/collisions_framework.hpp"
#include "physics/rect_bounding_box.hpp"

#include <initializer_list>
#include <vector>
#include <utility>

#include "defs/common.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
INCL_WARN


class SAT : public CollisionsFramework
{
public:
    bool collides(BoundingBox* a, BoundingBox* b) override;
    bool collides(RectBoundingBox& a, RectBoundingBox& b);  // NOLINT (runtime/references)
    bool collides(RectBoundingBox& a, const CircularBoundingBox& b);  // NOLINT (runtime/references)
    bool collides(const CircularBoundingBox& a, const CircularBoundingBox& b);  // NOLINT (runtime/references)

    inline static SAT* get()
    {
        if (!_instance)
        {
            _instance = new SAT();
        }

        return _instance;
    }

protected:
    SAT()
    {}

private:
    bool collides(const std::vector<glm::vec2>& axes, const BoundingBox* a, const BoundingBox* b);

private:
    static SAT* _instance;
};
