/* Copyright 2016 Guillem Pascual */

#include "map/cell.hpp"
#include "server/client.hpp"
#include "map/map-cluster/cluster_center.hpp"
#include "debug/debug.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"
#include "map/offset.hpp"
#include "map/quadtree.hpp"
#include "movement/motion_master.hpp"
#include "physics/bounding_box.hpp"

#include <algorithm>
#include <array>
#include <map>
#include <utility>
#include <vector>


Cell::Cell(Map* map, const Offset& offset) :
    _offset(std::move(offset)),
    _map(map),
    _clusterId(0),
    _lastUpdateKey(0)
{
    LOG(LOG_CELLS, "Created (%4d, %4d, %4d)", _offset.q(), _offset.r(), _offset.s());

    _broadcast = &_broadcastQueue1;
    _quadTree = new RadialQuadTree<5, 10>(offset.center(), cellSize_x + 10);
}

Cell::~Cell()
{
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
    std::vector<Cell*> results;
    // TODO(gpascualg): This is overestimating the size
    results.reserve(radius * radius * 6 + 1);

    int32_t q = _offset.q() + directions[4].q * radius;
    int32_t r = _offset.r() + directions[4].r * radius;

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

void Cell::update(uint64_t elapsed, int updateKey)
{
    // TODO(gpascualg): Check for better ways / clustering algo
    if (updateKey == _lastUpdateKey)
    {
        return;
    }
    _lastUpdateKey = updateKey;

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
        for (auto&& cell : _map->getSiblings(this))
        {
            if (cell->_quadTree->contains(updater->motionMaster()->position2D()))
            {
                cell->_quadTree->insert(updater);
            }
        }
    }
}

void Cell::physics(uint64_t elapsed, int updateKey)
{
    // TODO(gpascualg): Check for better ways / clustering algo
    if (updateKey == _lastUpdateKey)
    {
        return;
    }
    _lastUpdateKey = updateKey;

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

            if (e1->boundingBox()->overlaps(e2->boundingBox()))
            {
                // TODO(gpascualg): Apply forces to motionMaster, and possibly notify clients?
            }
        }
    }
}

void Cell::cleanup(uint64_t elapsed, int updateKey)
{
    // TODO(gpascualg): Check for better ways / clustering algo
    if (updateKey == _lastUpdateKey)
    {
        return;
    }
    _lastUpdateKey = updateKey;

    // Clear all broadcasts (should already be done!)
    // TODO(gpascualg): If a mob triggers a broadcast packet, it should be added to a "future" queue
    clearQueues();

    // Clear quadtree
    _quadTree->clear();
}

void Cell::processRequests(MapAwareEntity* entity)
{
    for (auto request : _requests)
    {
        if (request.who != entity)
        {
            if (request.type == RequestType::SPAWN)
            {
                // 0x0AAx are reserved packets
                auto packet = entity->spawnPacket();
                request.who->client()->send(packet);
            }
            else if (request.type == RequestType::DESPAWN)
            {
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

    _requests.push_back({ who, type });  // NOLINT(whitespace/braces)
}

void Cell::broadcast(boost::intrusive_ptr<Packet> packet)
{
    _broadcast->push_back(packet);
}

void Cell::clearQueues()
{
    auto& currentQueue = _broadcast == &_broadcastQueue1 ? _broadcastQueue2 : _broadcastQueue1;
    currentQueue.clear();
    _requests.clear();
}
