/* Copyright 2016 Guillem Pascual */

#pragma once

#include "offset.hpp"
#include "common.hpp"
#include "debug.hpp"

#include <array>
#include <list>
#include <utility>
#include <unordered_map>
#include <vector>

#include "intrusive.hpp"
#include <boost/intrusive_ptr.hpp>


class Cell;
class Cluster;
struct ClusterCenter;
class Map;
class MapAwareEntity;
class Packet;


class Cell
{
    friend class Map;
    friend class Cluster;

public:
    explicit Cell(Map* map, const Offset& offset);
    virtual ~Cell();

    inline const uint64_t hash() const { return _offset.hash(); }
    inline const Offset& offset() const { return _offset; }
    inline Map* map() const { return _map; }

    virtual void update(uint64_t elapsed, int updateKey);

    void broadcast(boost::intrusive_ptr<Packet> packet);
    void clearBroadcast();

    std::vector<Cell*> ring(uint16_t radius = 1);

protected:
    const Offset _offset;

    Map* _map;
    uint64_t _clusterId;
    int _lastUpdateKey;
    std::unordered_map<uint64_t /*id*/, MapAwareEntity*> _data;
    std::unordered_map<uint64_t /*id*/, MapAwareEntity*> _playerData;

    // TODO(gpascualg): Use some lockfree structure?
    std::list<boost::intrusive_ptr<Packet>> _broadcast;
};
