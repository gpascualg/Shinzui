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
#include <set>
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
    struct UpdateStructure
    {
        Cluster* cluster;
        uint64_t id;
        uint64_t elapsed;
        void (Cell::*fun)(uint64_t);
    };

public:
    virtual ~Cluster();

    void add(MapAwareEntity* entity, std::vector<Cell*> const& siblings);
    void remove(MapAwareEntity* entity);
    void update(uint64_t elapsed);
    void cleanup(uint64_t elapsed);
    void runScheduledOperations();

    void onCellCreated(Cell* cell);

    inline std::size_t size() { return _num_components; }

private:
    Cluster();

    void touch(Cell* cell);
    void touchWithNeighbours(Cell* cell);
    void connect(Cell* a, Cell* b);

private:
    threadpool11::Pool _pool;
    boost::lockfree::queue<ClusterOperation, boost::lockfree::capacity<4096>> _scheduledOperations;

    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Cell*>;
    Graph _graph;

    std::vector<Cell*> _verticesBuffer_1;
    std::vector<Cell*> _verticesBuffer_2;
    std::vector<Cell*>* _verticesCells;

    std::unordered_map<boost::graph_traits<Graph>::vertex_descriptor, std::vector<Cell*>> _cellsByCluster;
    std::unordered_map<Cell*, boost::graph_traits<Graph>::vertex_descriptor> _vertices;
    std::list<MapAwareEntity*> _keepers;

    uint16_t _num_components;
    std::vector<boost::graph_traits<Graph>::vertex_descriptor> _components;
};
