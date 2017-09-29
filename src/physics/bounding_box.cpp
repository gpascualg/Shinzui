/* Copyright 2016 Guillem Pascual */

#include "debug/debug.hpp"
#include "physics/bounding_box.hpp"
#include "movement/motion_master.hpp"

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>


BoundingBox::BoundingBox(MotionMaster* motionMaster, BoundingBoxType type) :
    _motionMaster(motionMaster),
    Type(type)
{}
