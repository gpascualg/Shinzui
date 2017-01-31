/* Copyright 2016 Guillem Pascual */

#pragma once

#include "offset.hpp"

#include <inttypes.h>


class Cell;
struct ClusterCenter;

enum class ClusterOperationType
{
    DESTROY,
    COMPACT
};

struct ClusterOperation
{
    ClusterOperationType type;
    uint64_t clusterId;
};
