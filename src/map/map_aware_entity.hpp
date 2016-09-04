#pragma once

#include <inttypes.h>


template <typename E> class Cell;

class MapAwareEntity
{
public:
    virtual uint32_t id() = 0;
    
    virtual void onAdded(Cell<MapAwareEntity>* cell) = 0;
    virtual void onRemoved(Cell<MapAwareEntity>* cell) = 0;
};
