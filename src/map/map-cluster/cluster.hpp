/* Copyright 2016 Guillem Pascual */

#pragma once

#include "debug/queue_with_size.hpp"

#include "defs/common.hpp"
#include "map/map-cluster/cluster_operation.hpp"

INCL_NOWARN
#include <threadpool11/threadpool11.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
INCL_WARN

#include <array>
#include <functional>
#include <list>
#include <map>
#include <unordered_set>
#include <vector>
#include <unordered_map>


class Cell;
struct ClusterCenter;
class Map;
class MapAwareEntity;

class Cluster
{
    friend class Map;

private:
    struct StallCell
    {
        bool isRegistered;
        bool isOnCooldown;
        uint64_t remaining;
        // TODO(gpasualg): Can we reduce it to uint32_t?
    };

public:
    virtual ~Cluster();

    void add(MapAwareEntity* entity, std::vector<Cell*> const& siblings);
    void remove(MapAwareEntity* entity);
    void update(uint64_t elapsed);
    void cleanup(uint64_t elapsed);
    void runScheduledOperations(uint64_t elapsed);

    void onCellCreated(Cell* cell);
    void checkStall(Cell* from, Cell* to);
    inline const std::unordered_set<Cell*>& cells() { return _stallCells; }

    inline std::size_t size() { return _num_components; }

private:
    Cluster();

    uint16_t processStallCells(uint64_t elapsed);

    bool touchWithNeighbours(Cell* cell, bool isStall = false);
    bool touch(Cell* cell, bool isStall = false);
    void connect(Cell* a, Cell* b);

private:
    threadpool11::Pool _pool;
    boost::lockfree::queue<ClusterOperation, boost::lockfree::capacity<4096>> _scheduledOperations;

    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Cell*>;
    Graph _graph;

    std::unordered_set<Cell*> _verticesBuffer_1;
    std::unordered_set<Cell*> _verticesBuffer_2;
    std::unordered_set<Cell*>* _currentCells;
    std::unordered_set<Cell*>* _oldCells;
    // TODO(gpascualg): Do we really need to keep track of so much lists??
    std::unordered_set<Cell*> _stallCells;

    uint16_t _numStall;
    uint16_t _numStallCandidates;

    std::unordered_map<boost::graph_traits<Graph>::vertex_descriptor, std::vector<Cell*>> _cellsByCluster;
    std::unordered_map<Cell*, boost::graph_traits<Graph>::vertex_descriptor> _vertices;
    std::list<MapAwareEntity*> _keepers;

    uint16_t _num_components;
    std::vector<boost::graph_traits<Graph>::vertex_descriptor> _components;
};

// template <> uint16_t Cluster::processStallCells(uint64_t elapsed, const std::vector<Cell*>& candidates);
// template <> uint16_t Cluster::processStallCells(uint64_t elapsed, const std::unordered_set<Cell*>& candidates);

