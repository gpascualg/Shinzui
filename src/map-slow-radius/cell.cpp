/* Copyright 2016 Guillem Pascual */

#include "cell.hpp"
#include "map.hpp"
#include "offset.hpp"
#include "cluster_center.hpp"
#include "map_aware_entity.hpp"
#include "debug.hpp"

#include <array>
#include <vector>
#include <map>

std::vector<Cell*> Cell::upperHalfSiblings(uint16_t deviation)
{
    return {  // NOLINT(whitespace/braces)
        _map->get(_offset.q() + 0 * deviation, _offset.r() - 1 * deviation),
        _map->get(_offset.q() + 1 * deviation, _offset.r() - 1 * deviation),
        _map->get(_offset.q() + 1 * deviation, _offset.r() + 0 * deviation)
    };
}

std::vector<Cell*> Cell::lowerHalfSiblings(uint16_t deviation)
{
    return {  // NOLINT(whitespace/braces)
        _map->get(_offset.q() + 0 * deviation, _offset.r() + 1 * deviation),
        _map->get(_offset.q() - 1 * deviation, _offset.r() + 1 * deviation),
        _map->get(_offset.q() - 1 * deviation, _offset.r() + 0 * deviation)
    };
}

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

void Cell::update(uint64_t elapsed)
{
    LOG(LOG_CLUSTERS, "\t\t(%d, %d)", _offset.q(), _offset.r());

    // Update players
    for (auto pair : _playerData)
    {
        pair.second->update(elapsed);
    }

    // Update mobs
    for (auto pair : _data)
    {
        pair.second->update(elapsed);
    }
}
