/* Copyright 2016 Guillem Pascual */

#include "map/cell.hpp"
#include "server/client.hpp"
#include "map/map-cluster/cluster.hpp"
#include "debug/debug.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"
#include "map/offset.hpp"
#include "map/quadtree.hpp"
#include "movement/motion_master.hpp"
#include "physics/bounding_box.hpp"
#include "server/server.hpp"

#include <algorithm>
#include <array>
#include <list>
#include <map>
#include <utility>
#include <vector>


Cell::Cell(Map* map, const Offset& offset) :
    _offset(std::move(offset)),
    _map(map),
    stall{false, false, 0}
{
    LOG(LOG_CELLS, "Created (%4d, %4d, %4d)", _offset.q(), _offset.r(), _offset.s());

    _broadcast = &_broadcastQueue1;
    _quadTree = new RadialQuadTree<MaxQuadrantEntities, MaxQuadtreeDepth>(offset.center(), cellSize_x + 10);

    Server::get()->onCellCreated(this);
}

Cell::~Cell()
{
    Server::get()->onCellDestroyed(this);

    delete _quadTree;
}

#undef min
#undef max

std::vector<Cell*> Cell::ring(uint16_t radius)
{
    std::vector<Cell*> results;
    results.reserve(radius * 6);

    int32_t q = _offset.q() + directions[4].q * radius;
    int32_t r = _offset.r() + directions[4].r * radius;
    Cell* cube = _map->get(q, r);

    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < radius; ++j)
        {
            results.emplace_back(cube);
            q += directions[i].q;
            r += directions[i].r;
            cube = _map->get(q, r);
        }
    }


    return results;
}

std::vector<Cell*> Cell::inRadius(uint16_t radius)
{
    // Reserve
    std::vector<Cell*> results;
    results.reserve(radius * radius * 6 + 1);

    // Fetch all cells (might not exist!)
    for (int dx = -radius; dx <= radius; ++dx)
    {
        int rmin = std::min<int>(radius, -dx + radius);
        for (int dy = std::max<int>(-radius, -dx - radius); dy <= rmin; ++dy)
        {
            results.push_back(_map->get(dx + _offset.q(), dy + _offset.r()));
        }
    }

    return results;
}

void Cell::update(uint64_t elapsed)
{
    auto& currentQueue = *_broadcast;
    _broadcast = _broadcast == &_broadcastQueue1 ? &_broadcastQueue2 : &_broadcastQueue1;

    // Update players
    for (auto pair : _entities)
    {
        auto updater = pair.second;
        auto client = updater->client();
        updater->update(elapsed);

        // If there is any packet, broadcast it!
        if (client)
        {
            for (auto packet : currentQueue)
            {
                client->send(packet);
            }
        }

        // Process map on spawn packets
        processRequests(updater);

        // Insert into quadtree
        _quadTree->insert(updater);

        // Try to insert into neighbours
        for (auto cell : _map->getSiblings(this))
        {
            if (cell && cell->_quadTree->contains(updater->motionMaster()->position2D()))
            {
                cell->_quadTree->insert(updater);
            }
        }
    }
}

void Cell::physics(uint64_t elapsed)
{
    // Collisions
    for (auto pair1 : _entities)
    {
        auto e1 = pair1.second;

        std::list<MapAwareEntity*> candidates;
        _quadTree->retrieve(candidates, e1->boundingBox()->asRect());

        for (auto e2 : candidates)
        {
            if (e1 == e2)
            {
                continue;
            }

            if (SAT::get()->collides(e1->boundingBox(), e2->boundingBox()))
            {
                // TODO(gpascualg): Apply forces to motionMaster, and possibly notify clients?
            }
        }
    }
}

void Cell::cleanup(uint64_t elapsed)
{
    // Clear all broadcasts (should already be done!)
    // TODO(gpascualg): If a mob triggers a broadcast packet, it should be added to a "future" queue
    clearQueues();

    // Clear quadtree
    _quadTree->clear();
}

void Cell::addEntity(MapAwareEntity* entity)
{
    _entities.emplace(entity->id(), entity);
    
    if (entity->client())
    {
        ++_clientsCount;
    }
}

void Cell::removeEntity(MapAwareEntity* entity)
{
    _entities.erase(entity->id());

    if (entity->client())
    {
        --_clientsCount;
    }
}

// TODO: If player spawns at the same time as a mob, a double spawn is sent

void Cell::processRequests(MapAwareEntity* entity)
{
    for (auto request : _requests)
    {
        if (request.who != entity)
        {
            if (request.type == RequestType::SPAWN)
            {
                LOG(LOG_SPAWNS, "(%d, %d) Serving SPAWN request from %" PRId64 " with entity %" PRId64, offset().q(), offset().r(), request.who->id(), entity->id());

                // 0x0AAx are reserved packets
                auto packet = entity->spawnPacket();
                request.who->client()->send(packet);
            }
            else if (request.type == RequestType::DESPAWN)
            {
                LOG(LOG_SPAWNS, "(%d, %d) Serving DESPAWN request from %" PRId64 " with entity %" PRId64, offset().q(), offset().r(), request.who->id(), entity->id());

                // 0x0AAx are reserved packets
                auto packet = entity->despawnPacket();
                request.who->client()->send(packet);
            }
        }
    }
}

void Cell::request(MapAwareEntity* who, RequestType type)
{
    // TODO(gpascualg): Macro/debug friendly assert
    assert(who->client());

    LOG(LOG_SPAWNS, "(%d, %d) Request from %" PRId64, offset().q(), offset().r(), who->id());
    _requests.push_back({ who, type });  // NOLINT(whitespace/braces)
}

void Cell::broadcast(boost::intrusive_ptr<Packet> packet)
{
    // There is nothing to broadcast if there is no plalyer at all
    if (_clientsCount > 0)
    {
        LOG(LOG_SPAWNS, "(%d, %d) Broadcast requested", offset().q(), offset().r());

        _broadcast->push_back(packet);
    }
}

void Cell::clearQueues()
{
    auto& currentQueue = _broadcast == &_broadcastQueue1 ? _broadcastQueue2 : _broadcastQueue1;
    currentQueue.clear();
    _requests.clear();
}
