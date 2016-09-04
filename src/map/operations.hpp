#pragma once

#include "offset.hpp"

#include <inttypes.h>


enum class MapOperationType
{
    ADD_ENTITY_CREATE,
    REMOVE_ENTITY,
    DESTROY
};

template <typename E>
struct MapOperation
{
    MapOperationType type;
    const Offset offset;
    E entity;
};

#include "detail/operations.hpp"
