/* Copyright 2016 Guillem Pascual */


#include "atomic_autoincrement.hpp"
#include "cell.hpp"
#include "cluster.hpp"
#include "cluster_center.hpp"
#include "cluster_operation.hpp"
#include "common.hpp"
#include "debug.hpp"
#include "map_aware_entity.hpp"
#include "offset.hpp"

#include <algorithm>
#include <set>
#include <vector>


Cluster::Cluster()
{
    _scheduledOperations = new QueueWithSize<ClusterOperation*>(2048);
}

Cluster::~Cluster()
{
    _updaterEntities.clear();
    delete _scheduledOperations;
}

void Cluster::update(uint64_t elapsed)
{
    // Try to update unique clusters only
    for (auto id : _uniqueIdsList)
    {
        // TODO(gpascualg): Launch thread per cluster
        updateCluster(new UpdateStructure{ this, id, elapsed });  // NOLINT(whitespace/braces)
    }
}

void Cluster::updateCluster(UpdateStructure* updateStructure)
{
    auto* cluster = updateStructure->cluster;
    auto id = updateStructure->id;
    auto elapsed = updateStructure->elapsed;

    // As long as they are different, race-conditions are of no importance
    auto updateKey = rand();  // NOLINT(runtime/threadsafe_fn)

    // Get all old nodes
    for (auto subId : cluster->_uniqueClusters[id])
    {
        // Get all entites
        auto queue = cluster->_updaterEntities[subId];

        // Update all entities
        for (auto* entity : queue)
        {
            // Update cell
            entity->cell()->update(elapsed, updateKey);

            // Update neighbour cells in radius 1
            for (auto* cell : entity->cell()->ring(1))
            {
                cell->update(elapsed, updateKey);
            }
        }
    }
}

void Cluster::runScheduledOperations()
{
    // TODO(gpascualg): Unique IDs compation at _uniqueClusters, _updaterEntities level
}

void Cluster::add(MapAwareEntity* entity, std::vector<Cell*> const& siblings)
{
    // Apply to current
    uint64_t uniqueClusterId = AtomicAutoIncrement<1>::get();
    entity->cell()->_clusterId = uniqueClusterId;
    _uniqueClusters[uniqueClusterId].emplace_back(uniqueClusterId);

    // Are neighours in a cluster already? Assign them either way
    for (Cell* cell : siblings)
    {
        // Get old id, if any (that is > 0)
        uint64_t oldId = cell->_clusterId;
        if (oldId > 0)
        {
            // Remove unique Ids if any
            auto it = _uniqueIdsList.find(oldId);
            if (it != _uniqueIdsList.end())
            {
                // Unify old ids into the new one
                _uniqueClusters[uniqueClusterId].emplace_back(oldId);
                _uniqueIdsList.erase(it);
            }
        }

        // Setup current cell
        cell->_clusterId = uniqueClusterId;
    }

    // Add updater entity
    _updaterEntities[uniqueClusterId].emplace_back(entity);
    _uniqueIdsList.emplace(uniqueClusterId);
}

void Cluster::remove(MapAwareEntity* entity)
{
    // Simply erase from the updaterEntities
    auto clusterId = entity->cell()->_clusterId;
    auto& entities = _updaterEntities[clusterId];
    entities.erase(std::find(entities.begin(), entities.end(), entity));
}
