#pragma once

#include <map_aware_entity.hpp>
#include <server.hpp>

#include <inttypes.h>

class Cell;

static uint32_t gID = 0;
class Entity : public MapAwareEntity
{
public:
    using MapAwareEntity::MapAwareEntity;

    uint32_t id() { return _id; }

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

    void update(uint64_t elapsed) override { MapAwareEntity::update(elapsed); }

    bool hasBeenAdded = false;
    bool hasBeenRemoved = false;

private:
    uint32_t _id = gID++;
};


class TestServer : public Server
{
public:
    using Server::Server;

    void handleRead(Client* client, const boost::system::error_code& error, size_t size){}
};
