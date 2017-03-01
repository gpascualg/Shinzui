/* Copyright 2016 Guillem Pascual */

#include "cell.hpp"
#include "client.hpp"
#include "cluster_center.hpp"
#include "debug.hpp"
#include "map.hpp"
#include "map_aware_entity.hpp"
#include "offset.hpp"

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
}

Cell::~Cell()
{}

std::vector<Cell*> Cell::ring(uint16_t radius)
{
    std::vector<Cell*> results;
    // results.resize(radius * 6, nullptr);

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

void Cell::update(uint64_t elapsed, int updateKey)
{
    if (updateKey == _lastUpdateKey)
    {
        return;
    }
    _lastUpdateKey = updateKey;

    // Update players
    for (auto pair : _playerData)
    {
        auto player = pair.second;
        auto client = player->client();
        player->update(elapsed);

        // If there is any packet, broadcast it!
        for (auto packet : _broadcast)
        {
            client->send(packet);
        }

        // Process map on spawn packets
        processRequests(player);
    }

    // Update mobs
    for (auto pair : _data)
    {
        auto entity = pair.second;
        entity->update(elapsed);

        // Process on spawn packets
        processRequests(entity);
    }

    // Clear all broadcasts (should already be done!)
    // TODO(gpascualg): If a mob triggers a broadcast packet, it should be added to a "future" queue
    clearQueues();
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
    _requests.push_back({ who, type });  // NOLINT(whitespace/braces)
}

void Cell::broadcast(boost::intrusive_ptr<Packet> packet)
{
    _broadcast.push_back(packet);
}

void Cell::clearQueues()
{
    _broadcast.clear();
    _requests.clear();
}
