#pragma once

#include "common.hpp"
#include "cluster_element.hpp"
#include "debug.hpp"

#include <array>
#include <vector>
#include <map>

template <typename E> class Map;
template <typename E> class Cell;

static int cellCount = 0;

template <typename E>
class Cell : public ClusterElement<Cell<E>*, 6>
{
    friend class Map<E>;

public:
    explicit Cell(Map<E>* map, const Offset&& offset) :
        _offset(std::move(offset)),
        _map(map)
    {
        int b;
        LOG(LOG_CELLS, "Created (%4d, %4d, %4d)", _offset.q(), _offset.r(), _offset.s(), b);
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
    
    std::vector<Cell<E>*> upperHalfSiblings(Cell<E>* cell);
    std::vector<Cell<E>*> lowerHalfSiblings(Cell<E>* cell);

private:
    const Offset _offset;

    Map<E>* _map;
    std::map<uint32_t /*id*/, E> _data;
};
