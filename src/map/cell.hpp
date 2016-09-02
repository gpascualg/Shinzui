#pragma once

#include "offset.hpp"
#include "cluster_element.hpp"
#include "debug.hpp"

#include <array>
#include <vector>
#include <map>

template <typename E> class Map;
template <typename E> class Cell;

static int cellCount = 0;

template <typename E>
class Cell : public ClusterElement<Cell<E>*>
{
    friend class Map<E>;

public:
    explicit Cell(Map<E>* map, const Offset&& offset) :
        ClusterElement<Cell<E>*>(),
        _offset(std::move(offset)),
        _map(map)
    {
        LOG(LOG_CELLS, "Created (%4d, %4d, %4d)", _offset.q(), _offset.r(), _offset.s());
        ++cellCount;
    }

    inline constexpr uint64_t hash()
    {
        return _offset.hash();
    }

    inline const Offset& offset() const
    {
        return _offset;
    }

    // TODO: Accept rotation instead of upper/lower
    std::vector<Cell<E>*> upperHalfSiblings(uint16_t deviation = 1);
    std::vector<Cell<E>*> lowerHalfSiblings(uint16_t deviation = 1);

    std::vector<Cell<E>*> ring(uint16_t radius = 1) override;

    void update(uint64_t elapsed) override
    {
        ClusterElement<Cell<E>*>::update(elapsed);
        LOG(LOG_CLUSTERS, "\t\t(%d, %d)", _offset.q(), _offset.r());
    }

private:
    const Offset _offset;

    Map<E>* _map;
    std::map<uint32_t /*id*/, E> _data;
};


template <typename E>
std::vector<Cell<E>*> Cell<E>::upperHalfSiblings(uint16_t deviation)
{
    return {
        _map->get(_offset.q() + 0 * deviation, _offset.r() - 1 * deviation),
        _map->get(_offset.q() + 1 * deviation, _offset.r() - 1 * deviation),
        _map->get(_offset.q() + 1 * deviation, _offset.r() + 0 * deviation)
    };
}

template <typename E>
std::vector<Cell<E>*> Cell<E>::lowerHalfSiblings(uint16_t deviation)
{
    return {
        _map->get(_offset.q() + 0 * deviation, _offset.r() + 1 * deviation),
        _map->get(_offset.q() - 1 * deviation, _offset.r() + 1 * deviation),
        _map->get(_offset.q() - 1 * deviation, _offset.r() + 0 * deviation)
    };
}

struct Direction
{
    int32_t q;
    int32_t r;
};
Direction directions[] = {
    {+1, -1}, {+1, +0}, {+0, +1},
    {-1, +1}, {-1, +0}, {+0, -1}
};

template <typename E>
std::vector<Cell<E>*> Cell<E>::ring(uint16_t radius)
{
    std::vector<Cell<E>*> results;
    //results.resize(radius * 6, nullptr);

    int32_t q = _offset.q() + directions[4].q * radius;
    int32_t r = _offset.r() + directions[4].r * radius;
    Cell<E>* cube = _map->get(q, r);

    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < radius; ++j)
        {
            results.emplace_back(cube);
            q += directions[i].q;
            r += directions[i].r;
            cube = _map->get(q, r);
        }
    }

    return results;
}
