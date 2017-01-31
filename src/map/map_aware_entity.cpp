/* Copyright 2016 Guillem Pascual */

#include "map_aware_entity.hpp"


void MapAwareEntity::onAdded(Cell* cell)
{
    _cell = cell;
}

void MapAwareEntity::onRemoved(Cell* cell)
{
    _cell = nullptr;
}
