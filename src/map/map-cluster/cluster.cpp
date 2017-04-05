/* Copyright 2016 Guillem Pascual */


#include "defs/atomic_autoincrement.hpp"
#include "map/cell.hpp"
#include "map/map-cluster/cluster.hpp"
#include "map/map-cluster/cluster_center.hpp"
#include "map/map-cluster/cluster_operation.hpp"
#include "defs/common.hpp"
#include "debug/debug.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"
#include "map/offset.hpp"

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
            // Update cell (Should be included in inRadius)
            // entity->cell()->update(elapsed, updateKey);

            // Update neighbour cells in radius 2
            auto cell = entity->cell();
            for (auto sibling : cell->inRadius(2))
            {
                if (sibling)
                {
                    sibling->update(elapsed, updateKey);
                }
            }
        }
    }
}

void Cluster::runScheduledOperations()
{
    // TODO(gpascualg): Unique IDs compaction at _uniqueClusters, _updaterEntities level
}

void Cluster::add(MapAwareEntity* entity, std::vector<Cell*> const& siblings)
{
    uint64_t uniqueClusterId = entity->cell()->_clusterId;

    // Is the cell at a cluster already?
    if (uniqueClusterId == 0)
    {
        // If not, assign a new cluster
        uniqueClusterId = AtomicAutoIncrement<1>::get();
        entity->cell()->_clusterId = uniqueClusterId;
        _uniqueClusters[uniqueClusterId].emplace_back(uniqueClusterId);

        LOG(LOG_CLUSTERS, "Create new cluster: %" PRId64, uniqueClusterId);
    }

    // Are neighours in a cluster already? Assign them either way
    for (Cell* cell : siblings)
    {
        // Get old id, if any (that is > 0) and different than current
        uint64_t oldId = cell->_clusterId;
        if (oldId > 0 && oldId != uniqueClusterId)
        {
            LOG(LOG_CLUSTERS, "\tMerged in %" PRId64 " <- %" PRId64, uniqueClusterId, oldId);

            // Remove unique Ids if any
            auto it = _uniqueIdsList.find(oldId);
            if (it != _uniqueIdsList.end())
            {
                // Unify old ids into the new one
                _uniqueClusters[uniqueClusterId].emplace_back(oldId);
                _uniqueIdsList.erase(it);
            }

            // TODO(gpascualg): What should we do with _updaterEntities[oldId]? - CRITICAL
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
    LOG(LOG_CLIENT_LIFECYCLE, "Removed updater entity %" PRId64, entity->id());

    // Simply erase from the updaterEntities
    auto clusterId = entity->cell()->_clusterId;
    auto& entities = _updaterEntities[clusterId];

    // TODO(gpascualg): Find the entity in whichever cluster it is (see oldId above) - CRITICAL
    entities.erase(std::find(entities.begin(), entities.end(), entity));

    // TODO(gpascualg): Place a dummy updateEntity to avoid instantly discarting the cell
    /*
    if (_updaterEntities[clusterId].empty() && entity->client())
    {
        auto updater = new DummyUpdater(-1);
        updater->triggerUpdater();
        updater->cell(entity->cell());

        _updaterEntities[clusterId].emplace_back(updater);
    }
    */
}
