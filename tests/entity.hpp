#pragma once

#include <map_aware_entity.hpp>

#include <inttypes.h>

class Cell;

static uint32_t gID = 0;
class Entity : public MapAwareEntity
{
public:
    uint32_t id() { return _id; }

    void onAdded(Cell* cell) { hasBeenAdded = true; }
    void onRemoved(Cell* cell) { hasBeenRemoved = true; }

    bool hasBeenAdded = false;
    bool hasBeenRemoved = false;

private:
    uint32_t _id = gID++;
};
