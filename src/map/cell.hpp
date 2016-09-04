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
    bool _siblingsDone = false;
    std::map<uint32_t /*id*/, E> _data;
};


#include "detail/cell.hpp"
