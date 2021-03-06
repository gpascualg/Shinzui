/* Copyright 2016 Guillem Pascual */

#include "cluster.hpp"
#include "offset.hpp"
#include "debug.hpp"
#include "common.hpp"
#include "cluster_center.hpp"
#include "cluster_operation.hpp"
#include "cell.hpp"

#include <algorithm>
#include <vector>


Cluster::Cluster()
{
    _scheduledOperations = new QueueWithSize<ClusterOperation*>(2048);
}

Cluster::~Cluster()
{
    for (auto& cluster : _uniqueClusters)
    {
        delete cluster;
    }

    for (auto& map : _cache)
    {
        map.second.clear();
    }

    _uniqueClusters.clear();
    _cache.clear();

    delete _scheduledOperations;
}

void Cluster::update(uint64_t elapsed)
{
    LOG(LOG_CLUSTERS, "... %d", _uniqueClusters.size());
    for (auto& cluster : _uniqueClusters)
    {
        LOG(LOG_CLUSTERS, "-] Updating cluster " FMT_PTR, (uintptr_t)cluster);

        // Propagate updates
        propagate(cluster->center, [cluster, elapsed, this](Cell* node) -> bool
        {
            if (node->_cluster == cluster)
            {
                node->update(elapsed);
                return true;
            }
            else
            {
                LOG(LOG_CLUSTERS, "Should be at cluster " FMT_PTR, (uintptr_t)node->_cluster);

                // TODO(gpascualg): Do more in depth tests into this
                if (!node->_cluster->mergeSignaled && !cluster->mergeSignaled)
                {
                    _scheduledOperations->push(new ClusterOperation {  // NOLINT(whitespace/braces)
                        ClusterOperationType::MERGE,
                        node->_cluster,
                        cluster
                    });  // NOLINT(whitespace/braces)

                    cluster->mergeSignaled = true;
                }
            }

            return false;
        });  // NOLINT(whitespace/braces)

        // TODO(gpascualg): Optimize cluster center
    }

    ++_fetchCurrent;
}

void Cluster::runScheduledOperations()
{
    ClusterOperation* operation;
    while (_scheduledOperations->pop(operation))
    {
        switch (operation->type)
        {
            case ClusterOperationType::MERGE:
            {
                auto cluster1 = operation->cluster1;
                auto cluster2 = operation->cluster2;
                auto iterator1 = std::find(_uniqueClusters.begin(), _uniqueClusters.end(), cluster1);
                auto iterator2 = std::find(_uniqueClusters.begin(), _uniqueClusters.end(), cluster2);

                LOG(LOG_CLUSTERS, FMT_PTR "\t" FMT_PTR "\n", (uintptr_t)cluster1, (uintptr_t)cluster2);

                if (iterator1 != _uniqueClusters.end() && iterator2 != _uniqueClusters.end())
                {
                    auto center1 = cluster1->center;
                    auto center2 = cluster2->center;
                    LOG(LOG_CLUSTERS, "\t>in (%u)\n", (uint32_t)_uniqueClusters.size());

                    // Move all from cluster 2 to cluster 1s
                    propagate(center2, [cluster2, cluster1](Cell* node) -> bool
                    {
                        if (node->_cluster == cluster2)
                        {
                            node->_cluster = cluster1;
                            return true;
                        }

                        return false;
                    });  // NOLINT(whitespace/braces)

                    // Remove cluster 2
                    _uniqueClusters.erase(iterator2);
                    LOG(LOG_CLUSTERS, "\t<out (%u)\n", (uint32_t)_uniqueClusters.size());
                }

                break;
            }

            case ClusterOperationType::RING_INVALIDATION:
                // We are not recreating clusters now because:
                //   a) Would trigger one rebuilt per ring invalidation
                //   b) Does not take profit of threading, whereas `update` does
                getRing(operation->cluster1->center, operation->radius, true, false);
                break;

            default:
                // TODO(gpascualg): Log unkown
                break;
        }

        delete operation;
    }
}

uint16_t Cluster::propagate(Cell* center, std::function<bool(Cell*)> fnc)
{
    uint32_t callCount = 0;
    uint16_t radius = 0;

    // Call at center
    fnc(center);

    // Radially propagate
    do
    {
        ++radius;
        callCount = 0;

        std::vector<Cell*>& nodes = getRing(center, radius);
        for (auto node : nodes)
        {
            if (node)
            {
                callCount += fnc(node);
            }
        }

        LOG(LOG_CLUSTERS, "Ring at %d = %d", radius, callCount);
    }
    while (callCount > 0);

    return radius;
}


std::vector<Cell*>& Cluster::getRing(Cell* center, uint16_t radius, bool invalidate, bool recreate)
{
    if (invalidate || _cache.find(center) == _cache.end() || _cache[center].find(radius) == _cache[center].end())
    {
        if (recreate)
        {
            _cache[center][radius] = center->ring(radius);
        }
        else
        {
            _cache[center].erase(radius);
            return _placeHolder;
        }
    }

    return _cache[center][radius];
}


void Cluster::add(Cell* node, std::vector<Cell*> const& siblings)
{
    // Create a cluster and assign the node
    _uniqueClusters.emplace_back(new ClusterCenter { node });  // NOLINT(whitespace/braces)
    node->_cluster = _uniqueClusters.back();

    // Setup neighbours cluster
    Cell* center = node->_cluster->center;
    for (auto sibling : siblings)
    {
        sibling->_cluster = node->_cluster;
    }

    // Create ring invalidation operations
    for (auto k : {-1, 0, 1})
    {
        _scheduledOperations->push(new ClusterOperation
        {
            ClusterOperationType::RING_INVALIDATION,
            node->_cluster,
            nullptr,
            node->offset().distance(center->offset()) + k
        });  // NOLINT(whitespace/braces)
    }
}
