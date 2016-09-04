#pragma once

#include <inttypes.h>


class Cell;

class MapAwareEntity
{
public:
    virtual uint32_t id() = 0;

    virtual void onAdded(Cell* cell) = 0;
    virtual void onRemoved(Cell* cell) = 0;
};
