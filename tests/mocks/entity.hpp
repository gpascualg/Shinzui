#pragma once

#include <map/map_aware_entity.hpp>

#include <inttypes.h>

class Cell;

static uint32_t gID = 0;
class Entity : public MapAwareEntity
{
public:
    using MapAwareEntity::MapAwareEntity;

    uint32_t id() { return _id; }

    inline Entity* asDefault()
    {
        setupBoundingBox({ {-0.5, -0.5}, {-0.5, 0.5}, {0.5, 0.5}, {0.5, -0.5} });
        return this;
    }

    std::vector<Cell*> onAdded(Cell* cell, Cell* old) override
    {
        hasBeenAdded = true;
        return MapAwareEntity::onAdded(cell, old);
    }

    std::vector<Cell*> onRemoved(Cell* cell, Cell* to) override
    {
        hasBeenRemoved = true;
        return MapAwareEntity::onRemoved(cell, to);
    }

    Packet* spawnPacket() override
    {
        return nullptr;
    }
    Packet* despawnPacket() override
    {
        return nullptr;
    }

    void update(uint64_t elapsed) override { MapAwareEntity::update(elapsed); }

    bool hasBeenAdded = false;
    bool hasBeenRemoved = false;

    void forceUpdater() { _isUpdater = true; }

private:
    uint32_t _id = gID++;
};
