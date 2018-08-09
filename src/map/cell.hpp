/* Copyright 2016 Guillem Pascual */

#pragma once

#include "map/offset.hpp"
#include "debug/debug.hpp"

#include <array>
#include <list>
#include <utility>
#include <unordered_map>
#include <vector>

#include "defs/common.hpp"

INCL_NOWARN
#include "defs/intrusive.hpp"
#include <boost/intrusive_ptr.hpp>
INCL_WARN


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

    struct StallInformation
    {
        bool isRegistered;
        bool isOnCooldown;
        uint64_t remaining;
    };

public:
    explicit Cell(Map* map, const Offset& offset);
    virtual ~Cell();

    inline const uint64_t hash() const { return _offset.hash(); }
    inline const Offset& offset() const { return _offset; }
    inline Map* map() const { return _map; }
    inline RadialQuadTree<MaxQuadrantEntities, MaxQuadtreeDepth>* quadtree() { return _quadTree; }

    virtual void update(uint64_t elapsed);
    virtual void physics(uint64_t elapsed);
    virtual void cleanup(uint64_t elapsed);

    void addEntity(MapAwareEntity* entity);
    void removeEntity(MapAwareEntity* entity);

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

    RadialQuadTree<MaxQuadrantEntities, MaxQuadtreeDepth>* _quadTree;
    std::unordered_map<uint64_t /*id*/, MapAwareEntity*> _entities;
    uint16_t _clientsCount;

    // Use double lists to avoid locking and/or non-desired cleanups
    std::list<boost::intrusive_ptr<Packet>> _broadcastQueue1;
    std::list<boost::intrusive_ptr<Packet>> _broadcastQueue2;
    std::list<boost::intrusive_ptr<Packet>>* _broadcast;

    std::list<Request> _requests;

public:
    // TODO(gpascualg): Better encapsulation for stall information?
    StallInformation stall;
};
