#include "cluster.hpp"
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
void Cluster<E>::update(uint64_t elapsed)
{
    LOG(LOG_CLUSTERS, "... %d", _uniqueClusters.size());
    for (auto& cluster : _uniqueClusters)
    {
        LOG(LOG_CLUSTERS, "-] Updating cluster " FMT_PTR " = %d", (uintptr_t)cluster, cluster->size());
        for (auto& center : *cluster)
        {
            // Propagate updates
            propagate(center, [cluster, elapsed](E node) -> bool {
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

        std::vector<E> nodes;
        if (_cache.find(center) != _cache.end() && _cache[center].find(radius) != _cache[center].end())
        {
            nodes = _cache[center][radius];
        }
        else
        {
            nodes = center->ring(radius);
            _cache[center][radius] = nodes;
        }

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
        if (_numClusters < MaxClusters)
        {
            _uniqueClusters.emplace_back(new std::vector<E> { (E)node });
            node->_cluster = _uniqueClusters.back();
        }
        else
        {
            auto uniqueCluster = *(_uniqueClusters.begin() + (int)(rand() / (float)RAND_MAX * MaxClusters));
            uniqueCluster->emplace_back((E)node);
            node->_cluster = uniqueCluster;
        }

        // Setup neighbours cluster
        for (auto sibling : siblings)
        {
            sibling->_cluster = node->_cluster;
        }
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
                    for (auto node : *currentCluster)
                    {
                        propagate(node, [firstCluster, currentCluster](auto node) -> bool {
                            if (node && node->_cluster == currentCluster)
                            {
                                node->_cluster = firstCluster;
                                return true;
                            }

                            return false;
                        });
                    }
                }
            }
        }

        // Setup neighbours cluster
        node->_cluster = firstCluster;
        for (auto sibling : siblings)
        {
            sibling->_cluster = firstCluster;
        }
    }

    // Invalidate the cache
    for (auto cluster : *node->_cluster)
    {
        _cache[cluster].erase(node->offset().distance(cluster->offset()));
    }
}
