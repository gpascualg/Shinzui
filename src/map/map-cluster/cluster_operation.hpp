/* Copyright 2016 Guillem Pascual */

#pragma once

#include "map/offset.hpp"

#include <inttypes.h>


class Cell;
struct ClusterCenter;

enum class ClusterOperationType
{
    KEEP,
    UNKEEP
};

struct ClusterOperation
{
    ClusterOperationType type;
    MapAwareEntity* entity;
};
