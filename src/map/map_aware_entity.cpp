/* Copyright 2016 Guillem Pascual */

#include "map_aware_entity.hpp"
#include "client.hpp"


MapAwareEntity::MapAwareEntity(uint64_t id, Client* client) :
    _client(client),
    _id(id),
    _cell(nullptr),
    _position{ 0, 0 }
{}

MapAwareEntity::~MapAwareEntity()
{}

void MapAwareEntity::onAdded(Cell* cell)
{
    _cell = cell;
}

void MapAwareEntity::onRemoved(Cell* cell)
{
    _cell = nullptr;
}
