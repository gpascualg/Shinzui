#pragma once

#include <map_aware_entity.hpp>

#include <inttypes.h>

class Cell;

static uint32_t gID = 0;
class Entity : public MapAwareEntity
{
public:
    using MapAwareEntity::MapAwareEntity;

    uint32_t id() { return _id; }

    void onAdded(Cell* cell, Cell* old) override { hasBeenAdded = true; MapAwareEntity::onAdded(cell, old); }
    void onRemoved(Cell* cell, Cell* to) override { hasBeenRemoved = true; MapAwareEntity::onRemoved(cell, to); }
    void update(uint64_t elapsed) override { MapAwareEntity::update(elapsed); }

    bool hasBeenAdded = false;
    bool hasBeenRemoved = false;

private:
    uint32_t _id = gID++;
};
