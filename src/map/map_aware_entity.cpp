/* Copyright 2016 Guillem Pascual */

#include "cell.hpp"
#include "client.hpp"
#include "map.hpp"
#include "map_aware_entity.hpp"
#include "motion_master.hpp"
#include "server.hpp"

#include <list>
#include <vector>


MapAwareEntity::MapAwareEntity(uint64_t id, Client* client) :
    _client(client),
    _id(id),
    _cell(nullptr)
{
    _motionMaster = new MotionMaster(this);
}

MapAwareEntity::~MapAwareEntity()
{
    delete _motionMaster;
}

void MapAwareEntity::update(uint64_t elapsed)
{
    _motionMaster->update(elapsed);
}

std::vector<Cell*> MapAwareEntity::onAdded(Cell* cell, Cell* old)
{
    _cell = cell;

    Packet* packet = spawnPacket();

    // Broadcast all packets
    auto newCells = cell->map()->getCellsExcluding(cell, old);
    Server::get()->map()->broadcast(newCells, packet, [this](Cell* cell)
        {
            if (client())
            {
                cell->request(this, RequestType::SPAWN);
            }
        }
    );  // NOLINT(whitespace/parens)

    return newCells;
}

std::vector<Cell*> MapAwareEntity::onRemoved(Cell* cell, Cell* to)
{
    _cell = nullptr;

    Packet* packet = despawnPacket();

    // Broadcast all packets
    auto oldCells = cell->map()->getCellsExcluding(cell, to);
    Server::get()->map()->broadcast(oldCells, packet, [this](Cell* cell)
        {
            if (client())
            {
                cell->request(this, RequestType::DESPAWN);
            }
        }
    );  // NOLINT(whitespace/parens)

    return oldCells;
}
