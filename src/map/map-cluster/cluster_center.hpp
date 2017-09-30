/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>
#include <algorithm>
#include <array>
#include <list>
#include <vector>

#include "defs/common.hpp"
#include "debug/debug.hpp"


class Cluster;
class Cell;
class BoundingBox;
class MapAwareEntity;

struct ClusterCenter
{
    Cell* center;
    uint16_t radius;
    std::list<MapAwareEntity*> updaters;

    explicit ClusterCenter(Cell* ctr, uint16_t rad) :
        center(ctr),
        radius(rad)
    {}
};
