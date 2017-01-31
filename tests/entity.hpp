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

    void onAdded(Cell* cell) override { hasBeenAdded = true; MapAwareEntity::onAdded(cell); }
    void onRemoved(Cell* cell) override { hasBeenRemoved = true; MapAwareEntity::onRemoved(cell);}
    void update(uint64_t elapsed) override { MapAwareEntity::update(elapsed); }

    bool hasBeenAdded = false;
    bool hasBeenRemoved = false;

private:
    uint32_t _id = gID++;
};
