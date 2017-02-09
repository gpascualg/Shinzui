/* Copyright 2016 Guillem Pascual */

#pragma once

#include "offset.hpp"
#include "boost/lockfree/lockfree_forward.hpp"
#include "boost/pool/pool_forward.hpp"

#include <inttypes.h>
#include <vector>
#include <unordered_map>

#include "intrusive.hpp"
#include <boost/intrusive_ptr.hpp>


class Cell;
class CellAllocator;
class Cluster;
class Map;
class MapAwareEntity;
struct MapOperation;


class Map
{
public:
    explicit Map(boost::object_pool<Cell>* cellAllocator = nullptr);
    Map(const Map& map) = delete;
    virtual ~Map();

    void update(uint64_t elapsed);
    void runScheduledOperations();

    // Broadcast operations
    void broadcastToSiblings(Cell* cell, boost::intrusive_ptr<Packet> packet);
    void broadcastExcluding(Cell* cell, Cell* exclude, boost::intrusive_ptr<Packet> packet);

    // Automated add/remove
    void onMove(MapAwareEntity* entity);

    // Schedules an ADD (and maybe CREATE) operations
    void addTo(MapAwareEntity* e, Cell* old);
    void addTo2D(int32_t x, int32_t y, MapAwareEntity* e, Cell* old);
    void addTo(int32_t q, int32_t r, MapAwareEntity* e, Cell* old);
    void addTo(const Offset&& offset, MapAwareEntity* e, Cell* old);
    void addTo(Cell* cell, MapAwareEntity* e, Cell* old);

    // Schedules an REMOVE operation
    void removeFrom(MapAwareEntity* e, Cell* to);
    void removeFrom2D(int32_t x, int32_t y, MapAwareEntity* e, Cell* to);
    void removeFrom(int32_t q, int32_t r, MapAwareEntity* e, Cell* to);
    void removeFrom(const Offset&& offset, MapAwareEntity* e, Cell* to);
    void removeFrom(Cell* cell, MapAwareEntity* e, Cell* to);

    // NOT thread-safe
    // Gets a cell from the map
    Cell* get(int32_t q, int32_t r);
    Cell* get(const Offset& offset);

    // NOT thread-safe
    // Gets or creates a cell from the map
    Cell* getOrCreate(int32_t q, int32_t r);
    Cell* getOrCreate(const Offset& offset);

    // NOT thread-safe
    // Creates siblings for a cell
    std::vector<Cell*> createSiblings(Cell* cell);
    std::vector<Cell*> getSiblings(Cell* cell);

    inline Cluster* cluster() { return _cluster; }
    inline uint32_t size() { return _cells.size(); }


private:
    boost::object_pool<Cell>* _cellAllocator;
    Cluster* _cluster;

    std::unordered_map<uint64_t /*hash*/, Cell*> _cells;
    boost::lockfree::queue<MapOperation*>* _scheduledOperations;
};
