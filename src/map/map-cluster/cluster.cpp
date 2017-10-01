/* Copyright 2016 Guillem Pascual */


#include "defs/atomic_autoincrement.hpp"
#include "defs/common.hpp"
#include "debug/debug.hpp"
#include "map/cell.hpp"
#include "map/map-cluster/cluster.hpp"
#include "map/map-cluster/cluster_center.hpp"
#include "map/map-cluster/cluster_operation.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"
#include "map/offset.hpp"
#include "movement/motion_master.hpp"
#include "physics/rect_bounding_box.hpp"
#include "server/server.hpp"

#include <algorithm>
#include <set>
#include <vector>


Cluster::Cluster()
{
    _scheduledOperations = new QueueWithSize<ClusterOperation*>(2048);
}

Cluster::~Cluster()
{
    for (auto center : _clusterCenters)
    {
        delete center.second;
    }

    _clusterCenters.clear();
    delete _scheduledOperations;
}

void Cluster::update(uint64_t elapsed)
{
    // Try to update unique clusters only
    for (auto id : _uniqueIdsList)
    {
        pool.postWork([elapsed]() { updateCluster({ this, id, elapsed, &Cell::update }); }); // NOLINT(whitespace/braces)
    }

    pool.waitAll();

    // Update physics
    for (auto id : _uniqueIdsList)
    {
        pool.postWork([elapsed]() { updateCluster({ this, id, elapsed, &Cell::physics }); }); // NOLINT(whitespace/braces)
    }

    pool.waitAll();
}


void Cluster::cleanup(uint64_t elapsed)
{
    // Cleanup
    for (auto id : _uniqueIdsList)
    {
        pool.postWork([elapsed]() { updateCluster({ this, id, elapsed, &Cell::cleanup }); }); // NOLINT(whitespace/braces)
    }
    
    pool.waitAll();
}

void Cluster::updateCluster(UpdateStructure&& updateStructure)
{
    auto* cluster = updateStructure.cluster;
    auto id = updateStructure.id;
    auto elapsed = updateStructure.elapsed;
    
    // Expand center
    auto center = cluster->_clusterCenters[id];
    for (auto cell : center->center->inRadius(center->radius))
    {
        // No need for the cell to exist!
        if (cell)
        {
            (cell->*updateStructure.fun)(elapsed);
        }
    }
}

void Cluster::runScheduledOperations()
{
    // TODO(gpascualg): Unique IDs compaction at _uniqueClusters, _updaterEntities level
}

void Cluster::add(MapAwareEntity* entity, std::vector<Cell*> const& siblings)
{
    auto affectedCell = entity->cell();
    uint64_t uniqueClusterId = affectedCell->_clusterId;
    uint64_t oldUniqueClusterid = uniqueClusterId;

    // Is the cell at a cluster already?
    if (uniqueClusterId == 0)
    {
        // If not, assign a new cluster
        uniqueClusterId = AtomicAutoIncrement<1>::get();
        affectedCell->_clusterId = uniqueClusterId;

        LOG(LOG_CLUSTERS, "Create new cluster: %" PRId64, uniqueClusterId);
    }

    // Do we have a cell center?
    ClusterCenter* center = nullptr;
    auto it = _clusterCenters.find(uniqueClusterId);
    if (it == _clusterCenters.end())
    {
        _uniqueIdsList.emplace(uniqueClusterId);

        center = new ClusterCenter{ affectedCell, 2 };
        center->updaters.push_back(entity);
        _clusterCenters.emplace(uniqueClusterId, center);
    }
    else
    {
        center = it->second;
        center->updaters.push_back(entity);
    }

    // Are neighours in a cluster already? Assign them either way
    for (Cell* cell : siblings)
    {
        // Get old id, if any (that is > 0) and different than current
        uint64_t oldId = cell->_clusterId;
        if (oldId > 0 && oldId != uniqueClusterId)
        {
            // Remove unique Ids if any
            auto it = _uniqueIdsList.find(oldId);
            if (it != _uniqueIdsList.end())
            {
                LOG(LOG_CLUSTERS, "\tMerged in %" PRId64 " <- %" PRId64, uniqueClusterId, oldId);

                auto oldCenter = _clusterCenters[oldId];
                center->updaters.splice(center->updaters.end(), oldCenter->updaters);

                // Clean unique id
                _uniqueIdsList.erase(it);

                // Clean cluster center
                delete oldCenter;
                _clusterCenters.erase(oldId);
            }
        }

        // Setup current cell
        cell->_clusterId = uniqueClusterId;
    }

    // Update cluster center now 
    if (it != _clusterCenters.end())
    {
        // Update center unless the cluster already existed
        if (oldUniqueClusterid != uniqueClusterId)
        {
            // Calculate center based on existing entities
            glm::vec2 newCenter = { 0, 0 };

            for (auto entity : center->updaters)
            {
                newCenter += entity->motionMaster()->position2D();
            }

            newCenter /= center->updaters.size();
            center->center = Server::get()->map()->getOrCreate(offsetOf(newCenter.x, newCenter.y));
        }

        // Is this cell (or neighbours +2) further from the center?
        int dist = center->center->offset().distance(affectedCell->offset()) + 2;
        center->radius = std::max(center->radius, static_cast<uint16_t>(dist));

        LOG(LOG_CLUSTERS, "\tExpanded cluster %" PRId64 " to %d", uniqueClusterId, center->radius);
    }
}

void Cluster::remove(MapAwareEntity* entity)
{
    LOG(LOG_CLIENT_LIFECYCLE, "Removed updater entity %" PRId64, entity->id());

    // Simply erase from the updaterEntities
    auto clusterId = entity->cell()->_clusterId;
    auto& entities = _clusterCenters[clusterId]->updaters;

    // Remove updater
    entities.erase(std::find(entities.begin(), entities.end(), entity));

    // TODO(gpascualg): Update center and radius

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
