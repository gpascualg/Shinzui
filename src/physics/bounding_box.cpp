/* Copyright 2016 Guillem Pascual */

#include "debug/debug.hpp"
#include "physics/bounding_box.hpp"
#include "movement/motion_master.hpp"

#include <algorithm>

#include "defs/common.hpp"

INCL_NOWARN
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
INCL_WARN


BoundingBox::BoundingBox(BoundingBoxType type) :
    Type(type)
{}

BoundingBox::~BoundingBox()
{}
