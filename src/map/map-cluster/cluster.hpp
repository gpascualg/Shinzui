/* Copyright 2016 Guillem Pascual */

#pragma once

#include "queue_with_size.hpp"

#include <array>
#include <functional>
#include <map>
#include <set>
#include <vector>
#include <unordered_map>


struct ClusterCenter;
struct ClusterOperation;
class Cell;
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
    };

public:
    virtual ~Cluster();

    void add(MapAwareEntity* entity, std::vector<Cell*> const& siblings);
    void remove(MapAwareEntity* entity);
    void update(uint64_t elapsed);
    void runScheduledOperations();

    static void updateCluster(UpdateStructure* updateStructure);

    inline std::size_t size() { return _uniqueIdsList.size(); }

private:
    Cluster();

private:
    uint32_t _numClusters = 0;
    uint32_t _fetchCurrent = 0;

    QueueWithSize<ClusterOperation*>* _scheduledOperations;

    // Unique Cluster ID to old IDs
    std::unordered_map<uint64_t, std::vector<uint64_t>> _uniqueClusters;
    // List of unique cluster IDs
    std::set<uint64_t> _uniqueIdsList;
    // Cluster ID to MapAwareEntity
    std::unordered_map<uint64_t, std::vector<MapAwareEntity*>> _updaterEntities;
};
