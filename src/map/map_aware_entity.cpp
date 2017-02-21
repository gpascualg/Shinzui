/* Copyright 2016 Guillem Pascual */

#include "client.hpp"
#include "map_aware_entity.hpp"
#include "motion_master.hpp"


MapAwareEntity::MapAwareEntity(uint64_t id, Client* client) :
    _client(client),
    _id(id),
    _cell(nullptr)
{
    _motionMaster = new MotionMaster(this);
}

MapAwareEntity::~MapAwareEntity()
{
    delete _motionMaster;
}

void MapAwareEntity::update(uint64_t elapsed)
{
    _motionMaster->update(elapsed);
}

void MapAwareEntity::onAdded(Cell* cell, Cell* old)
{
    _cell = cell;
}

void MapAwareEntity::onRemoved(Cell* cell, Cell* to)
{
    _cell = nullptr;
}
