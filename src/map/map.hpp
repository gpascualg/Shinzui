#pragma once

#include "common.hpp"

#include <inttypes.h>
#include <vector>
#include <unordered_map>

template <typename E> class Cell;

template <typename E>
class Map
{
public:

    Map(int32_t x, int32_t y, uint32_t dx, uint32_t dy);

    Cell<E>* addTo2D(int16_t x, int16_t y, E e);
    Cell<E>* addTo(int16_t q, int16_t r, E e);
    Cell<E>* addTo(const Offset&& offset, E e);

    Cell<E>* get(int16_t q, int16_t r);
    Cell<E>* get(const Offset& offset);

    Cell<E>* getOrCreate(int16_t q, int16_t r, bool siblings = true);
    Cell<E>* getOrCreate(const Offset& offset, bool siblings = true);

    std::vector<Cell<E>*> createSiblings(Cell<E>* cell);

    inline int size() { return _cells.size(); }

private:
    int32_t _x;
    int32_t _y;
    uint32_t _dx;
    uint32_t _dy;

    std::unordered_map<uint64_t /*hash*/, Cell<E>*> _cells;
};

#include "map_i.hpp"
