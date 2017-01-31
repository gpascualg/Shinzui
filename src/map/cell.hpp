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
        _map(map),
        _clusterId(0),
        _lastUpdateKey(0)
    {
        LOG(LOG_CELLS, "Created (%4d, %4d, %4d)", _offset.q(), _offset.r(), _offset.s());
    }

    virtual ~Cell()
    {}

    inline const uint64_t hash() const { return _offset.hash(); }
    inline const Offset& offset() const { return _offset; }

    virtual void update(uint64_t elapsed, int updateKey);

    std::vector<Cell*> ring(uint16_t radius = 1);

protected:
    const Offset _offset;

    Map* _map;
    uint64_t _clusterId;
    int _lastUpdateKey;
    std::unordered_map<uint32_t /*id*/, MapAwareEntity*> _data;
    std::unordered_map<uint32_t /*id*/, MapAwareEntity*> _playerData;
};
