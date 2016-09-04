#pragma once

#include "../cluster.hpp"
#include "offset.hpp"
#include "debug.hpp"
#include "common.hpp"

#include <algorithm>


template <typename E>
Cluster<E>* Cluster<E>::_instance = nullptr;


template <typename E>
Cluster<E>::Cluster()
{}

template <typename E>
Cluster<E>::~Cluster()
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

template <typename E>
void Cluster<E>::update(uint64_t elapsed)
{
    LOG(LOG_CLUSTERS, "... %d", _uniqueClusters.size());
    for (auto& cluster : _uniqueClusters)
    {
        LOG(LOG_CLUSTERS, "-] Updating cluster " FMT_PTR, (uintptr_t)cluster);

        // Propagate updates
        propagate(cluster->center, [cluster, elapsed](E node) -> bool {
            if (node->_cluster == cluster)
            {
                node->update(elapsed);
                return true;
            }
            LOG(LOG_CLUSTERS, "Should be at cluster " FMT_PTR, (uintptr_t)node->_cluster);
            return false;
        });

        // TODO: Optimize cluster center
    }

    ++_fetchCurrent;
}

template <typename E>
uint16_t Cluster<E>::propagate(E center, std::function<bool(E)> fnc)
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

        std::vector<E>& nodes = getRing(center, radius);
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

template <typename E>
std::vector<E>& Cluster<E>::getRing(E center, uint16_t radius, bool invalidate, bool recreate)
{
    std::vector<E> nodes;
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

template <typename E>
void Cluster<E>::add(E node, std::vector<E>& siblings)
{
    // Find if there is any nonNullSibling cluster
    std::vector<E> nonNullSiblings(siblings.size());
    auto lastInserted = std::copy_if(siblings.begin(), siblings.end(), nonNullSiblings.begin(), [this](E s) {
        return s->_cluster != nullptr;
    });
    nonNullSiblings.resize(std::distance(nonNullSiblings.begin(), lastInserted));

    // It's a new node
    LOG(LOG_CLUSTERS, "Has nonNullSiblings: %d", nonNullSiblings.empty());
    if (nonNullSiblings.empty())
    {
        // Create cluster if it can be created
        _uniqueClusters.emplace_back(new ClusterCenter<E> { (E)node });
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
    E center = node->_cluster->center;
    for (auto sibling : siblings)
    {
        sibling->_cluster = node->_cluster;
        // TODO: We should recreate ring on the end, not now nor at next update
        getRing(center, sibling->offset().distance(center->offset()), true, false);
    }
    // TODO: We should recreate ring on the end, not now
    getRing(center, node->offset().distance(center->offset()), true, false);
}