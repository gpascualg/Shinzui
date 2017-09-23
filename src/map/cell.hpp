/* Copyright 2016 Guillem Pascual */

#pragma once

#include "map/offset.hpp"
#include "defs/common.hpp"
#include "debug/debug.hpp"

#include <array>
#include <list>
#include <utility>
#include <unordered_map>
#include <vector>

#include "defs/intrusive.hpp"
#include <boost/intrusive_ptr.hpp>


class Cell;
class Cluster;
struct ClusterCenter;
class Map;
class MapAwareEntity;
class Packet;
template <int E, int D>
class RadialQuadTree;

enum class RequestType
{
    SPAWN,
    DESPAWN
};

struct Request
{
    MapAwareEntity* who;
    RequestType type;
};

class Cell
{
    friend class Map;
    friend class Cluster;

    static constexpr const int MaxQuadrantEntities = 5;
    static constexpr const int MaxQuadtreeDepth = 10;

public:
    explicit Cell(Map* map, const Offset& offset);
    virtual ~Cell();

    inline const uint64_t hash() const { return _offset.hash(); }
    inline const Offset& offset() const { return _offset; }
    inline Map* map() const { return _map; }
    inline RadialQuadTree<MaxQuadrantEntities, MaxQuadtreeDepth>* quadtree() { return _quadTree; }

    virtual void update(uint64_t elapsed, int updateKey);
    virtual void physics(uint64_t elapsed, int updateKey);
    virtual void cleanup(uint64_t elapsed, int updateKey);

    void request(MapAwareEntity* who, RequestType type);
    void broadcast(boost::intrusive_ptr<Packet> packet);
    void clearQueues();

    std::vector<Cell*> inRadius(uint16_t radius = 1);
    std::vector<Cell*> ring(uint16_t radius = 1);

private:
    void processRequests(MapAwareEntity* entity);

protected:
    const Offset _offset;

    Map* _map;
    uint64_t _clusterId;
    int _lastUpdateKey;

    RadialQuadTree<MaxQuadrantEntities, MaxQuadtreeDepth>* _quadTree;
    std::unordered_map<uint64_t /*id*/, MapAwareEntity*> _entities;

    // TODO(gpascualg): Use double lists to avoid locking and/or non-desired cleanups
    std::list<boost::intrusive_ptr<Packet>> _broadcastQueue1;
    std::list<boost::intrusive_ptr<Packet>> _broadcastQueue2;
    std::list<boost::intrusive_ptr<Packet>>* _broadcast;

    std::list<Request> _requests;
};
