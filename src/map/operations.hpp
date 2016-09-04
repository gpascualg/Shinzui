#pragma once

#include "offset.hpp"

#include <inttypes.h>


class MapAwareEntity;

enum class MapOperationType
{
    ADD_ENTITY_CREATE,
    REMOVE_ENTITY,
    DESTROY
};

struct MapOperation
{
    MapOperationType type;
    const Offset offset;
    MapAwareEntity* entity;
};

#include "detail/operations.hpp"
