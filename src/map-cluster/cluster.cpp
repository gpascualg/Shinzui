#include "cluster.hpp"
#include "offset.hpp"
#include "debug.hpp"
#include "common.hpp"
#include "cluster_element.hpp"
#include "cell.hpp"

#include <algorithm>


Cluster::Cluster()
{}

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
}

void Cluster::update(uint64_t elapsed)
{
    LOG(LOG_CLUSTERS, "... %d", _uniqueClusters.size());
    for (auto& cluster : _uniqueClusters)
    {
        LOG(LOG_CLUSTERS, "-] Updating cluster " FMT_PTR, (uintptr_t)cluster);

        // Propagate updates
        propagate(cluster->center, [cluster, elapsed](Cell* node) -> bool {
            if (node->_cluster == cluster)
            {
                node->update(elapsed);
                return true;
            }
            // TODO: This else should be a merge operation! One cluster has found another one! :D
            LOG(LOG_CLUSTERS, "Should be at cluster " FMT_PTR, (uintptr_t)node->_cluster);
            return false;
        });

        // TODO: Optimize cluster center
    }

    ++_fetchCurrent;
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


void Cluster::add(Cell* node, std::vector<Cell*>& siblings)
{
    // TODO: Once merging is detected on update, all this logic could be removed

    // Find if there is any nonNullSibling cluster
    // Find also if the ring at radius 2 has any cell, in that case, merge
    std::vector<Cell*> nonNullSiblings(6 + 12);
    // Siblings
    auto lastInserted = std::copy_if(siblings.begin(), siblings.end(), nonNullSiblings.begin(), [this](Cell* s) {
        return s && s->_cluster != nullptr;
    });
    // Radius 2
    auto radius2 = node->ring(2);
    lastInserted = std::copy_if(radius2.begin(), radius2.end(), lastInserted, [this](Cell* s){
        return s && s->_cluster != nullptr;
    });
    // Resize
    nonNullSiblings.resize(std::distance(nonNullSiblings.begin(), lastInserted));

    // It's a new node
    LOG(LOG_CLUSTERS, "Has nonNullSiblings: %d", nonNullSiblings.empty());
    if (nonNullSiblings.empty())
    {
        // Create cluster if it can be created
        _uniqueClusters.emplace_back(new ClusterCenter { node });
        node->_cluster = _uniqueClusters.back();
    }
    else
    {
        // Check if there is a conflict in clusters
        auto firstCluster = nonNullSiblings[0]->_cluster;
        for (auto sibling : nonNullSiblings)
        {
            auto currentCluster = sibling->_cluster;

            if (currentCluster != firstCluster)
            {
                LOG(LOG_CLUSTERS, "SHOULD MERGE NOW " FMT_PTR " with " FMT_PTR, (uintptr_t)currentCluster, (uintptr_t)firstCluster);

                // Has it been already removed?
                auto it = std::find(_uniqueClusters.begin(), _uniqueClusters.end(), currentCluster);

                if (it != _uniqueClusters.end())
                {
                    // Remove from unique clusters
                    _uniqueClusters.erase(it);

                    // Update cluster references
                    propagate(currentCluster->center, [firstCluster, currentCluster](auto node) -> bool {
                        if (node && node->_cluster == currentCluster)
                        {
                            node->_cluster = firstCluster;
                            return true;
                        }

                        return false;
                    });

                    // Delete cluster center
                    delete currentCluster;
                }
            }
        }

        // Setup neighbours cluster
        node->_cluster = firstCluster;
    }

    // Setup neighbours cluster and invalidate ring cache
    Cell* center = node->_cluster->center;
    for (auto sibling : siblings)
    {
        sibling->_cluster = node->_cluster;
        // TODO: We should recreate ring on the end, not now nor at next update
        getRing(center, sibling->offset().distance(center->offset()), true, false);
    }
    // TODO: We should recreate ring on the end, not now
    getRing(center, node->offset().distance(center->offset()), true, false);
}
