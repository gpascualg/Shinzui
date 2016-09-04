#pragma once

#include "offset.hpp"
#include "cluster_element.hpp"
#include "debug.hpp"

#include <array>
#include <vector>
#include <map>


class Map;
class Cell;
class MapAwareEntity;

static int cellCount = 0;

class Cell : public ClusterElement<Cell*>
{
    friend class Map;

public:
    explicit Cell(Map* map, const Offset&& offset) :
        ClusterElement<Cell*>(),
        _offset(std::move(offset)),
        _map(map)
    {
        LOG(LOG_CELLS, "Created (%4d, %4d, %4d)", _offset.q(), _offset.r(), _offset.s());
        ++cellCount;
    }

    inline const uint64_t hash() const
    {
        return _offset.hash();
    }

    inline const Offset& offset() const
    {
        return _offset;
    }

    // TODO: Accept rotation instead of upper/lower
    std::vector<Cell*> upperHalfSiblings(uint16_t deviation = 1);
    std::vector<Cell*> lowerHalfSiblings(uint16_t deviation = 1);

    std::vector<Cell*> ring(uint16_t radius = 1) override;

    void update(uint64_t elapsed) override
    {
        ClusterElement<Cell*>::update(elapsed);
        LOG(LOG_CLUSTERS, "\t\t(%d, %d)", _offset.q(), _offset.r());
    }

private:
    const Offset _offset;

    Map* _map;
    bool _siblingsDone = false;
    std::map<uint32_t /*id*/, MapAwareEntity*> _data;
};
