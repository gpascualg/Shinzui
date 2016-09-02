#pragma once

#include <inttypes.h>
#include <algorithm>
#include <array>
#include <vector>

#include "cluster.hpp"
#include "debug.hpp"


template <typename E> class Cluster;

static int updateCount = 0;

template <typename E>
struct ClusterCenter
{
    E center;
};

template <typename E>
class ClusterElement
{
    friend class Cluster<E>;

public:
    ClusterElement()
    {}

    virtual void update(uint64_t elapsed)
    {
        LOG(LOG_CLUSTERS, "\t-] Updating " FMT_PTR, (uintptr_t)this);
        ++updateCount;
    }

    //virtual std::vector<E> upperHalfSiblings(uint16_t deviation = 1) = 0;
    //virtual std::vector<E> lowerHalfSiblings(uint16_t deviation = 1) = 0;

    virtual std::vector<E> ring(uint16_t radius = 1) = 0;

private:
    ClusterCenter<E>* _cluster = nullptr;
};
