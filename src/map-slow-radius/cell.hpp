/* Copyright 2016 Guillem Pascual */

#pragma once

#include "offset.hpp"
#include "common.hpp"
#include "debug.hpp"

#include <array>
#include <vector>
#include <unordered_map>
#include <utility>


class Cluster;
struct ClusterCenter;
class Map;
class Cell;
class MapAwareEntity;


class Cell
{
    friend class Map;
    friend class Cluster;

public:
    explicit Cell(Map* map, const Offset& offset) :
        _offset(std::move(offset)),
        _map(map)
    {
        LOG(LOG_CELLS, "Created (%4d, %4d, %4d)", _offset.q(), _offset.r(), _offset.s());
    }

    virtual ~Cell()
    {}

    inline const uint64_t hash() const
    {
        return _offset.hash();
    }

    inline const Offset& offset() const
    {
        return _offset;
    }

    // TODO(gpascualg): Accept rotation instead of upper/lower
    std::vector<Cell*> upperHalfSiblings(uint16_t deviation = 1);
    std::vector<Cell*> lowerHalfSiblings(uint16_t deviation = 1);

    std::vector<Cell*> ring(uint16_t radius = 1);

    void update(uint64_t elapsed);

private:
    const Offset _offset;

    Map* _map;
    bool _siblingsDone = false;
    std::unordered_map<uint32_t /*id*/, MapAwareEntity*> _data;
    std::unordered_map<uint32_t /*id*/, MapAwareEntity*> _playerData;

    ClusterCenter* _cluster = nullptr;
};
