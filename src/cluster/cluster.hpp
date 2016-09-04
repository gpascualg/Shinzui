#pragma once

#include <array>
#include <functional>
#include <map>
#include <vector>
#include <unordered_map>

template <typename E>
class ClusterElement;

template <typename E>
class ClusterCenter;


// Could we generalize this? :(
constexpr uint32_t MaxClusters = 12;

template <typename E>
class Cluster
{
    friend class ClusterElement<E>;

public:
    static Cluster<E>* get()
    {
        if (!_instance)
        {
            _instance = new Cluster();
        }
        return _instance;
    }

    void add(E node, std::vector<E>& siblings);
    void update(uint64_t elapsed);

private:
    Cluster();
    virtual ~Cluster();

    uint16_t propagate(E center, std::function<bool(E)> fnc);
    std::vector<E>& getRing(E center, uint16_t radius, bool invalidate = false, bool recreate = true);

private:
    static Cluster<E>* _instance;

    uint32_t _numClusters = 0;
    uint32_t _fetchCurrent = 0;

    std::vector<ClusterCenter<E>*> _uniqueClusters;
    std::unordered_map<E, std::unordered_map<uint16_t, std::vector<E>>> _cache;
    std::vector<E> _placeHolder;
};


#include "detail/cluster.hpp"
